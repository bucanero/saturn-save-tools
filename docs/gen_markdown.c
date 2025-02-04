#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SAVE_TYPE   "SEGA Saturn"
#define SAVE_FOLDER "."

void save_main(char* path, char* title)
{
    FILE * fp;
    FILE * out;
    char * line = NULL;
    char * pos;
    size_t len = 0;
    ssize_t read;

    chdir(path);
    fp = fopen("saves.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    out = fopen("README.md", "w");

    fprintf(out, "---\nlayout: default\ntitle: \"%s\"\nparent: " SAVE_TYPE " Saves\npermalink: " SAVE_FOLDER "/%s/\n---\n", title, path);
    fprintf(out, "# %s\n\n## " SAVE_TYPE " Saves\n\n`%s`\n\n| SAROO | .BUP | Description |\n|------|----------|-------------|\n", title, path);

    while ((read = getline(&line, &len, fp)) != -1) {
		line[read-1] = 0;
		if (line[read-2] == 0x0D) line[read-2]=0;
		pos = strchr(line, '=');
		if (!pos) continue;
		*pos = 0;
		*strchr(line, '.') = 0;
        fprintf(out, "| [%s.SRO](%s.SRO){: .btn .btn-purple } | [%s.BUP](%s.BUP){: .btn .btn-purple } | %s |\n", line, line, line, line, ++pos);
    }

    fclose(out);
    fclose(fp);
    if (line)
        free(line);

    chdir("..");
}

int main(int argc, char** argv)
{
    FILE * fp;
    char * line = NULL;
    char * pos;
    size_t len = 0;
    ssize_t read;

    fp = fopen("games.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    printf("---\nlayout: default\ntitle: " SAVE_TYPE " Saves\npermalink: " SAVE_FOLDER "/\nhas_children: true\nhas_toc: false\n---\n");
    printf("# Games\n\n| Game | Region |\n|------|----------|\n");

    while ((read = getline(&line, &len, fp)) != -1) {
		line[read-1] = 0;
		pos = strchr(line, '=');
		*pos++ = 0;
        printf("| [%s](%s/) | %.3s |\n", pos, line, strchr(pos, '(')+1);
        save_main(line, pos);
    }

    fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS);
}
