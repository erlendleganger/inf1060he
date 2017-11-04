#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char* argv[]) {

    if(argc < 4) {
        printf("USAGE: %s <infile> <outfile> <short/long>\n", argv[0]);
        return 0;
    }

    FILE* in_file = fopen(argv[1], "r");
    FILE* out_file = fopen(argv[2], "w+");

    srand(time(NULL));

    char buf[64000] = { 0 };
    int written_total = 0, loops = 0;
    for(;;) {
        unsigned int chars_to_read;
        if(argv[3][0] == 'l') {
            chars_to_read = (rand() % 54378) + 1;
        } else {
            chars_to_read = (rand() % 255) + 1;
        }

        size_t ret = fread(buf, 1, chars_to_read, in_file);
        if (ret != chars_to_read) {
            if(!feof(in_file)) {
                printf("Not EOF! Error occured\n");
                perror("fread()");
                exit(1);
            }
            break;
        }

        if(rand() % 2) {
            fwrite("O", 1, 1, out_file);
        } else {
            fwrite("E", 1, 1, out_file);
        }
        fwrite(&chars_to_read, sizeof(unsigned int), 1, out_file);
        unsigned int written = fwrite(buf, 1, chars_to_read, out_file);
        if(written != chars_to_read) {
            printf("%d %d\n", chars_to_read, written);
            perror("fwrite()");
            exit(1);
        }
        written_total += written;
        loops++;
        printf("Status: %5d >> %7d (%4d)\n", written, written_total, loops);
    }

    fclose(out_file);
    fclose(in_file);
    return 0;
}

