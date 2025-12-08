/*
╔════════════════════════════════════════════════════════════════════════════╗
║                                                                            ║
║              KramerScript – A Language About Nothing™                      ║
║                      Native C Implementation                               ║
║                                                                            ║
║  An esoteric HTTP server generator language using only Seinfeld quotes.   ║
║  "These pretzels are making me thirsty!"                                  ║
║                                                                            ║
╚════════════════════════════════════════════════════════════════════════════╝
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_ROUTES 64
#define MAX_ROUTE_PATH 256
#define MAX_RESPONSE_BODY 4096
#define MAX_LOG_MESSAGES 32
#define MAX_LOG_LENGTH 512
#define BUFFER_SIZE 8192
#define MAX_TOKENS 1024
#define TOKEN_VALUE_LEN 512
#define MAX_BITS 64
#define MAX_BIT_NAME 64
#define MAX_BIT_BODY 2048

/* Token types */
typedef enum {
    TOKEN_START,
    TOKEN_END,
    TOKEN_SERVER_START,
    TOKEN_ROUTE_START,
    TOKEN_SERVE,
    TOKEN_RBRACE,
    TOKEN_LOG,
    TOKEN_SLIDE_IN,
    TOKEN_BIT_START,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char value[TOKEN_VALUE_LEN];
} Token;

typedef struct {
    char path[MAX_ROUTE_PATH];
    int status;
    char body[MAX_RESPONSE_BODY];
} Route;

typedef struct {
    char name[MAX_BIT_NAME];
    char body[MAX_BIT_BODY];
} Bit;

typedef struct {
    char host[256];
    int port;
    Route routes[MAX_ROUTES];
    int route_count;
    Bit bits[MAX_BITS];
    int bit_count;
    char logs[MAX_LOG_MESSAGES][MAX_LOG_LENGTH];
    int log_count;
} Program;

typedef struct {
    char *data;
    size_t length;
    size_t pos;
} Lexer;

/* Global state */
static Program *g_program = NULL;
static volatile int server_running = 1;

/* Kramer quotes */
const char *kramer_quotes[] = {
    "Giddy up!",
    "These pretzels are making me thirsty!",
    "I'm out there Jerry and I'm loving every minute of it!",
    "Master of your domain!",
    "I'm a freak!",
    "The bro! The manssiere!",
    "I'm making a salad.",
    "Coffee's for closers!",
    "Let's go to the diner.",
    "Hellooo!",
    "I'm sliding in!",
    "You know, we're living in a society!",
    "That's gold Jerry, gold!",
    "Hello Newman!",
    "Serenity now!",
    "Not that there's anything wrong with that!",
    "Double dip!",
    "It's the little things.",
    "Yada yada yada.",
    "Moops!",
    "I'm back, baby!"
};

int num_quotes = sizeof(kramer_quotes) / sizeof(kramer_quotes[0]);

/* Utility functions */
void to_upper(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

char *trim(char *str) {
    while (isspace((unsigned char)*str)) str++;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
    return str;
}

int string_contains(const char *str, const char *substr) {
    return strstr(str, substr) != NULL;
}

void print_random_quote(void) {
    srand((unsigned int)time(NULL));
    printf("%s\n", kramer_quotes[rand() % num_quotes]);
}

/* Bit (function) management */
const char *get_bit(const char *name) {
    if (!g_program) return NULL;
    
    for (int i = 0; i < g_program->bit_count; i++) {
        if (strcmp(g_program->bits[i].name, name) == 0) {
            return g_program->bits[i].body;
        }
    }
    return NULL;
}

/* Expand bit calls in response body */
void expand_bits(char *input, char *output, size_t out_size) {
    size_t in_pos = 0;
    size_t out_pos = 0;
    size_t in_len = strlen(input);
    
    while (in_pos < in_len && out_pos < out_size - 1) {
        /* Look for {{BIT_NAME}} pattern */
        if (input[in_pos] == '{' && in_pos + 1 < in_len && input[in_pos + 1] == '{') {
            size_t bit_start = in_pos + 2;
            size_t bit_pos = bit_start;
            
            /* Find closing }} */
            while (bit_pos + 1 < in_len && !(input[bit_pos] == '}' && input[bit_pos + 1] == '}')) {
                bit_pos++;
            }
            
            if (bit_pos + 1 < in_len && input[bit_pos] == '}' && input[bit_pos + 1] == '}') {
                /* Extract bit name */
                char bit_name[MAX_BIT_NAME] = {0};
                size_t name_len = bit_pos - bit_start;
                if (name_len < MAX_BIT_NAME) {
                    strncpy(bit_name, input + bit_start, name_len);
                    bit_name[name_len] = '\0';
                    
                    /* Get bit content */
                    const char *bit_content = get_bit(bit_name);
                    if (bit_content) {
                        size_t content_len = strlen(bit_content);
                        if (out_pos + content_len < out_size - 1) {
                            strcpy(output + out_pos, bit_content);
                            out_pos += content_len;
                        }
                    }
                    
                    in_pos = bit_pos + 2;
                    continue;
                }
            }
        }
        
        /* Regular character */
        output[out_pos++] = input[in_pos++];
    }
    
    output[out_pos] = '\0';
}

/* Lexer */
Lexer *lexer_new(const char *source) {
    Lexer *lex = malloc(sizeof(Lexer));
    lex->length = strlen(source);
    lex->data = malloc(lex->length + 1);
    strcpy(lex->data, source);
    lex->pos = 0;
    return lex;
}

void lexer_free(Lexer *lex) {
    free(lex->data);
    free(lex);
}

void lexer_skip_whitespace(Lexer *lex) {
    while (lex->pos < lex->length && isspace((unsigned char)lex->data[lex->pos])) {
        lex->pos++;
    }
}

int lexer_match(Lexer *lex, const char *pattern, char *output) {
    lexer_skip_whitespace(lex);
    size_t pattern_len = strlen(pattern);
    
    if (lex->pos + pattern_len > lex->length) {
        return 0;
    }
    
    /* Build uppercase version of upcoming text */
    char temp_upper[TOKEN_VALUE_LEN] = {0};
    for (size_t i = 0; i < pattern_len && lex->pos + i < lex->length; i++) {
        temp_upper[i] = toupper((unsigned char)lex->data[lex->pos + i]);
    }
    
    if (strncmp(temp_upper, pattern, pattern_len) == 0) {
        /* Copy actual characters (preserving case) */
        for (size_t i = 0; i < pattern_len; i++) {
            output[i] = lex->data[lex->pos + i];
        }
        output[pattern_len] = '\0';
        lex->pos += pattern_len;
        return 1;
    }
    
    return 0;
}

int lexer_match_pattern(Lexer *lex, const char *pattern_desc, char *output) {
    lexer_skip_whitespace(lex);
    
    if (strcmp(pattern_desc, "STRING") == 0) {
        if (lex->data[lex->pos] != '"') return 0;
        lex->pos++;
        int len = 0;
        while (lex->pos < lex->length && lex->data[lex->pos] != '"' && len < TOKEN_VALUE_LEN - 1) {
            output[len++] = lex->data[lex->pos++];
        }
        output[len] = '\0';
        if (lex->pos < lex->length) lex->pos++; /* skip closing quote */
        return 1;
    }
    
    if (strcmp(pattern_desc, "NUMBER") == 0) {
        int len = 0;
        while (lex->pos < lex->length && isdigit((unsigned char)lex->data[lex->pos]) && len < TOKEN_VALUE_LEN - 1) {
            output[len++] = lex->data[lex->pos++];
        }
        output[len] = '\0';
        return len > 0;
    }
    
    return 0;
}

int lexer_match_serve(Lexer *lex, int *status, char *body) {
    lexer_skip_whitespace(lex);
    size_t start = lex->pos;
    
    /* Check for "SERVE" keyword */
    if (lex->pos + 5 > lex->length) return 0;
    
    char temp_upper[6];
    for (int i = 0; i < 5; i++) {
        temp_upper[i] = toupper((unsigned char)lex->data[lex->pos + i]);
    }
    temp_upper[5] = '\0';
    
    if (strcmp(temp_upper, "SERVE") != 0) {
        return 0;
    }
    
    lex->pos += 5;
    lexer_skip_whitespace(lex);
    
    /* Parse status code */
    char num[16] = {0};
    if (!lexer_match_pattern(lex, "NUMBER", num)) {
        lex->pos = start;
        return 0;
    }
    *status = atoi(num);
    lexer_skip_whitespace(lex);
    
    /* Parse optional body string */
    if (lex->pos < lex->length && lex->data[lex->pos] == '"') {
        if (!lexer_match_pattern(lex, "STRING", body)) {
            body[0] = '\0';
        }
    } else {
        body[0] = '\0';
    }
    
    return 1;
}

/* Parser */
int parse_program(const char *source, Program *prog) {
    Lexer *lex = lexer_new(source);
    
    /* Check for YO JERRY! */
    lexer_skip_whitespace(lex);
    char temp[TOKEN_VALUE_LEN];
    if (!lexer_match(lex, "YO JERRY!", temp)) {
        fprintf(stderr, "Error: Program must start with 'YO JERRY!'\n");
        lexer_free(lex);
        return 0;
    }
    
    /* Parse BIT definitions */
    while (lex->pos < lex->length) {
        lexer_skip_whitespace(lex);
        
        /* Peek ahead to see if this is a BIT definition */
        size_t saved_pos = lex->pos;
        if (!lexer_match(lex, "BIT", temp)) {
            lex->pos = saved_pos;
            break;
        }
        
        lexer_skip_whitespace(lex);
        char bit_name[MAX_BIT_NAME];
        if (!lexer_match_pattern(lex, "STRING", bit_name)) {
            fprintf(stderr, "Error: BIT requires name\n");
            lexer_free(lex);
            return 0;
        }
        
        lexer_skip_whitespace(lex);
        if (!lexer_match(lex, "{", temp)) {
            fprintf(stderr, "Error: Expected '{' after BIT name\n");
            lexer_free(lex);
            return 0;
        }
        
        /* Parse bit body - everything until closing brace */
        char bit_body[MAX_BIT_BODY] = {0};
        int body_len = 0;
        int brace_count = 1;
        
        while (lex->pos < lex->length && brace_count > 0 && body_len < MAX_BIT_BODY - 1) {
            if (lex->data[lex->pos] == '{') brace_count++;
            else if (lex->data[lex->pos] == '}') brace_count--;
            
            if (brace_count > 0) {
                bit_body[body_len++] = lex->data[lex->pos];
            }
            lex->pos++;
        }
        bit_body[body_len] = '\0';
        
        /* Store bit */
        if (prog->bit_count < MAX_BITS) {
            strcpy(prog->bits[prog->bit_count].name, bit_name);
            strcpy(prog->bits[prog->bit_count].body, bit_body);
            prog->bit_count++;
        }
    }
    
    /* Parse server block */
    lexer_skip_whitespace(lex);
    if (lexer_match(lex, "SERVER", temp)) {
        lexer_skip_whitespace(lex);
        char host_port[256];
        if (!lexer_match_pattern(lex, "STRING", host_port)) {
            fprintf(stderr, "Error: SERVER requires host:port\n");
            lexer_free(lex);
            return 0;
        }
        
        /* Parse host and port */
        char *colon = strchr(host_port, ':');
        if (colon) {
            strncpy(prog->host, host_port, colon - host_port);
            prog->host[colon - host_port] = '\0';
            prog->port = atoi(colon + 1);
        } else {
            strcpy(prog->host, host_port);
            prog->port = 8080;
        }
        
        lexer_skip_whitespace(lex);
        if (!lexer_match(lex, "{", temp)) {
            fprintf(stderr, "Error: Expected '{' after SERVER\n");
            lexer_free(lex);
            return 0;
        }
        
        /* Parse routes */
        while (lex->pos < lex->length) {
            lexer_skip_whitespace(lex);
            if (lexer_match(lex, "}", temp)) break;
            
            if (lexer_match(lex, "ROUTE", temp)) {
                lexer_skip_whitespace(lex);
                char path[MAX_ROUTE_PATH];
                if (!lexer_match_pattern(lex, "STRING", path)) {
                    fprintf(stderr, "Error: ROUTE requires path\n");
                    lexer_free(lex);
                    return 0;
                }
                
                lexer_skip_whitespace(lex);
                if (!lexer_match(lex, "{", temp)) {
                    fprintf(stderr, "Error: Expected '{' after ROUTE path\n");
                    lexer_free(lex);
                    return 0;
                }
                
                int status = 200;
                char body[MAX_RESPONSE_BODY] = {0};
                
                lexer_skip_whitespace(lex);
                if (lexer_match_serve(lex, &status, body)) {
                    /* Add route */
                    if (prog->route_count < MAX_ROUTES) {
                        strcpy(prog->routes[prog->route_count].path, path);
                        prog->routes[prog->route_count].status = status;
                        strcpy(prog->routes[prog->route_count].body, body);
                        prog->route_count++;
                    }
                }
                
                lexer_skip_whitespace(lex);
                if (!lexer_match(lex, "}", temp)) {
                    fprintf(stderr, "Error: Expected '}' after ROUTE body\n");
                    lexer_free(lex);
                    return 0;
                }
            } else {
                lex->pos++;
            }
        }
    }
    
    /* Parse LOG statements */
    while (lex->pos < lex->length) {
        lexer_skip_whitespace(lex);
        
        if (lexer_match(lex, "NOINE!", temp)) break;
        
        char message[MAX_LOG_LENGTH];
        if (lex->data[lex->pos] == '"') {
            lex->pos++;
            int len = 0;
            while (lex->pos < lex->length && lex->data[lex->pos] != '"' && len < MAX_LOG_LENGTH - 1) {
                message[len++] = lex->data[lex->pos++];
            }
            message[len] = '\0';
            if (lex->pos < lex->length) lex->pos++;
            
            lexer_skip_whitespace(lex);
            if (lexer_match(lex, "LOG", temp)) {
                if (prog->log_count < MAX_LOG_MESSAGES) {
                    strcpy(prog->logs[prog->log_count], message);
                    prog->log_count++;
                }
            }
        } else {
            lex->pos++;
        }
    }
    
    lexer_free(lex);
    return 1;
}

/* HTTP Server */
void send_http_response(int client_fd, int status, const char *body) {
    char response[8192];
    const char *status_text = "OK";
    
    if (status == 404) status_text = "Not Found";
    else if (status == 403) status_text = "Forbidden";
    else if (status == 418) status_text = "I'm a teapot";
    
    int body_len = strlen(body);
    snprintf(response, sizeof(response),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: text/plain\r\n"
             "Content-Length: %d\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             status, status_text, body_len, body);
    
    write(client_fd, response, strlen(response));
}

void send_file_response(int client_fd, const char *filepath) {
    struct stat st;
    if (stat(filepath, &st) == -1) {
        send_http_response(client_fd, 404, "File not found!");
        return;
    }
    
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        send_http_response(client_fd, 403, "Permission denied!");
        return;
    }
    
    char response[2048];
    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %lld\r\n"
             "Connection: close\r\n"
             "\r\n",
             (long long)st.st_size);
    
    write(client_fd, response, strlen(response));
    
    char buffer[4096];
    ssize_t bytes;
    while ((bytes = read(fd, buffer, sizeof(buffer))) > 0) {
        write(client_fd, buffer, bytes);
    }
    close(fd);
}

void handle_client(int client_fd) {
    char request[BUFFER_SIZE] = {0};
    ssize_t bytes_read = read(client_fd, request, sizeof(request) - 1);
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }
    
    /* Parse request path */
    char method[16], path[256], version[16];
    int parsed = sscanf(request, "%15s %255s %15s", method, path, version);
    if (parsed < 2) {
        send_http_response(client_fd, 400, "Bad request!");
        close(client_fd);
        return;
    }
    
    /* Check routes */
    for (int i = 0; i < g_program->route_count; i++) {
        if (strcmp(g_program->routes[i].path, path) == 0) {
            /* Expand bits in route body */
            char expanded_body[MAX_RESPONSE_BODY] = {0};
            expand_bits(g_program->routes[i].body, expanded_body, sizeof(expanded_body));
            send_http_response(client_fd, g_program->routes[i].status, expanded_body);
            close(client_fd);
            return;
        }
    }
    
    /* Try to serve from docs folder */
    char filepath[512];
    if (strcmp(path, "/") == 0) {
        strcpy(filepath, "./docs/index.html");
    } else {
        snprintf(filepath, sizeof(filepath), "./docs%s", path);
    }
    
    struct stat st;
    if (stat(filepath, &st) == 0 && S_ISREG(st.st_mode)) {
        send_file_response(client_fd, filepath);
    } else {
        send_http_response(client_fd, 404, "Not found!");
    }
    
    close(client_fd);
}

void *server_thread(void *arg __attribute__((unused))) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return NULL;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    /* Set socket to non-blocking mode */
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(g_program->port);
    inet_aton(g_program->host, &addr.sin_addr);
    
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(server_fd);
        return NULL;
    }
    
    listen(server_fd, 5);
    
    printf("These pretzels are making me thirsty!\n");
    printf("Giddy up! Server running on %s:%d\n", g_program->host, g_program->port);
    fflush(stdout);
    
    while (server_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd == -1) {
            /* Non-blocking accept returned no connection; sleep briefly */
            usleep(100000);  /* 100ms */
        } else {
            handle_client(client_fd);
        }
    }
    
    close(server_fd);
    return NULL;
}

void signal_handler(int sig __attribute__((unused))) {
    server_running = 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_random_quote();
        return 0;
    }
    
    if (strlen(argv[1]) < 7 || strcmp(argv[1] + strlen(argv[1]) - 7, ".kramer") != 0) {
        fprintf(stderr, "These aren't Kramer files, Jerry!\n");
        return 1;
    }
    
    /* Read file */
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Error: File '%s' not found, you dummy!\n", argv[1]);
        return 1;
    }
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *source = malloc(size + 1);
    fread(source, 1, size, fp);
    source[size] = '\0';
    fclose(fp);
    
    /* Parse program */
    g_program = calloc(1, sizeof(Program));
    if (!parse_program(source, g_program)) {
        fprintf(stderr, "Parse error!\n");
        free(source);
        free(g_program);
        return 1;
    }
    
    /* Print logs */
    for (int i = 0; i < g_program->log_count; i++) {
        printf("%s\n", g_program->logs[i]);
    }
    
    /* Start server if configured */
    if (g_program->port > 0) {
        signal(SIGINT, signal_handler);
        
        pthread_t thread;
        pthread_create(&thread, NULL, server_thread, NULL);
        
        /* Wait for server thread to finish */
        pthread_join(thread, NULL);
        
        printf("\nNOINE!\n");
    }
    
    free(source);
    free(g_program);
    return 0;
}
