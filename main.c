#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
 
#define SIZE 5
#define MAX_MESSAGE 205   
#define MAX_LINE 1024     
 
typedef struct {
    char grid[SIZE][SIZE];
    int row[26];
    int col[26];
} Square;
 
 
//build input string
static char *normalize(const Square *sq, const char *s)
{
    size_t len = strlen(s);
 
    //string cannot be longer than original
    char *out = malloc(len + 1);
 
    if (!out)
        return NULL;
    //store character number
    size_t n = 0;
 
    for (size_t i = 0; i < len; i++) {
 
        unsigned char uc = (unsigned char)s[i];
 
        if (!isalpha(uc))
            continue;
 
        char c = (char)toupper(uc);
 
        
        //if character isn't present in the key table.
        if (sq->row[c - 'A'] == -1)
            continue;
 
        out[n++] = c;
    }
 
    out[n] = '\0';
 
    return out;
}
 
static void transform_pair(
    const Square *sq,
    char *a,
    char *b,
    int dir
)
 
{
    //find each letter in table
    int ra = sq->row[*a - 'A'];
    int ca = sq->col[*a - 'A'];
 
    int rb = sq->row[*b - 'A'];
    int cb = sq->col[*b - 'A'];
 
    // if same row shift horizontally 
    if (ra == rb) {
 
        ca = (ca + dir + SIZE) % SIZE;
        cb = (cb + dir + SIZE) % SIZE;
 
    }
 
    
     //Same same column move vertically.
    else if (ca == cb) {
 
        ra = (ra + dir + SIZE) % SIZE;
        rb = (rb + dir + SIZE) % SIZE;
 
    }
 
        else {
 
        int temp = ca;
        ca = cb;
        cb = temp;
    }
 
    *a = sq->grid[ra][ca];
    *b = sq->grid[rb][cb];
}
 
 
static char *encrypt(const Square *sq, const char *plain)
{
    char *clean = normalize(sq, plain);
 
    if (!clean)
        return NULL;
 
    size_t len = strlen(clean);
 
    
    // Sort into pairs
    if (len % 2 != 0) {
        free(clean);
        return NULL;
    }
 
    char *out = malloc(len + 1);
 
    if (!out) {
        free(clean);
        return NULL;
    }
 
    for (size_t i = 0; i < len; i += 2) {
 
        char a = clean[i];
        char b = clean[i + 1];
 
        transform_pair(sq, &a, &b, 1); //1 defines direction
 
        out[i] = a;
        out[i + 1] = b;
    }
 
    out[len] = '\0';
 
    free(clean);
 
    return out;
}
//clean and repair in reverse
static char *decrypt(const Square *sq, const char *cipher)
{
    char *out = normalize(sq, cipher);
 
    if (!out)
        return NULL;
 
    size_t len = strlen(out);
 
    // Ciphertext must also contain pairs.
    if (len % 2 != 0) {
        free(out);
        return NULL;
    }
 
    for (size_t i = 0; i < len; i += 2) {
        transform_pair(sq, &out[i], &out[i + 1], -1);
    }
 
    return out;
}
 
 
//mark every letter as "not in the table"
static void init_square(Square *sq)
{
    for (int i = 0; i < 26; i++) {
        sq->row[i] = -1;
        sq->col[i] = -1;
    }
}
 
//place one letter in the next free cell, skipping letters already placed
static void add_letter(Square *sq, int *pos, char c)
{
    c = (char)toupper((unsigned char)c);
 
    //J shares a cell with I
    if (c == 'J')
        c = 'I';
 
    //already in the table
    if (sq->row[c - 'A'] != -1)
        return;
 
    int r = *pos / SIZE;
    int col = *pos % SIZE;
 
    sq->grid[r][col] = c;
    sq->row[c - 'A'] = r;
    sq->col[c - 'A'] = col;
 
    (*pos)++;
}
 
//table starts with user input keyword followed by rest of alphabet
static void build_square(Square *sq, const char *key)
{
    int pos = 0;
 
    init_square(sq);
 
    for (const char *p = key; *p; p++) {
        if (isalpha((unsigned char)*p))
            add_letter(sq, &pos, *p);
    }
 
    for (char c = 'A'; c <= 'Z'; c++)
        add_letter(sq, &pos, c);
 
    sq->row['J' - 'A'] = sq->row['I' - 'A'];
    sq->col['J' - 'A'] = sq->col['I' - 'A'];
}
 
static void print_square(const Square *sq)
{
    puts("\nKey table (I and J share a cell):");
 
    for (int r = 0; r < SIZE; r++) {
        printf("  ");
        for (int c = 0; c < SIZE; c++)
            printf("%c ", sq->grid[r][c]);
        printf("\n");
    }
}
  
//prompts
static bool read_line(const char *prompt, char *buf, size_t size)
{
    printf("%s", prompt);
    fflush(stdout);
 
    if (!fgets(buf, (int)size, stdin))
        return false;
 
    char *nl = strchr(buf, '\n');
 
    if (nl) {
        *nl = '\0';
    } else {
        //buffer protect
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }
 
    return true;
}
 
static void run_interactive(void)
{
    Square sq;
    char line[MAX_LINE];
 
    if (!read_line("Enter a keyword: ", line, sizeof(line)))
        return;
 
    build_square(&sq, line);
    print_square(&sq);
 
    while (true) {
 
        if (!read_line("\n(E)ncrypt, (D)ecrypt, (K)ey change, (Q)uit: ",
                       line, sizeof(line)))
            break;
 
        char choice = (char)toupper((unsigned char)line[0]);
 
        if (choice == 'Q')
            break;
 
        if (choice == 'K') {
            if (!read_line("Enter a new keyword: ", line, sizeof(line)))
                break;
            build_square(&sq, line);
            print_square(&sq);
            continue;
        }
 
        if (choice != 'E' && choice != 'D') {
            puts("Please enter E, D, K or Q.");
            continue;
        }
 
        if (!read_line("Message: ", line, sizeof(line)))
            break;
 
        char *answer = (choice == 'E') ? encrypt(&sq, line)
                                       : decrypt(&sq, line);
 
        if (answer == NULL) {

            puts("Can't process that message: it needs an even number of letters.");
        } else if (answer[0] == '\0') {
            puts("No letters to process.");
            free(answer);
        } else {
            printf("Result: %s\n", answer);
            free(answer);
        }
    }
 
    puts("Goodbye.");
}
  
static void run_batch(void)
{
    Square sq;
 
    init_square(&sq);
 
    //Read 5x5 key table
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
 
            scanf(" %c", &sq.grid[r][c]);
 
            char ch = sq.grid[r][c];
 
            sq.row[ch - 'A'] = r;
            sq.col[ch - 'A'] = c;
        }
    }
 
    // Read action.
    char action[20];
 
    scanf("%19s", action);
 
    //Read number of messages.
    int count;
 
    scanf("%d", &count);
 
    //Consume newline after count.
    getchar();
 
     //Process each message immediately.
    for (int i = 0; i < count; i++) {
 
        char message[MAX_MESSAGE];
 
        if (!fgets(message, sizeof(message), stdin))
            break;
 
        //Remove trailing newline.
        message[strcspn(message, "\n")] = '\0';
 
        char *answer;
 
        if (strcmp(action, "ENCRYPT") == 0) {
            answer = encrypt(&sq, message);
        } else {
            answer = decrypt(&sq, message);
        }
 
        if (answer == NULL) {
            printf("DUD\n");
        } else {
            printf("%s\n", answer);
            free(answer);
        }
    }
}
 
int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "--batch") == 0)
        run_batch();
    else
        run_interactive();
 
    return 0;
}
