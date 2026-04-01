/*
 * bxrd_prompt.c
 * Minimal, secure zsh prompt binary.
 * No external dependencies. No dynamic input eval. No shell injection surface.
 *
 * Segments:
 *   LEFT:  [venv?] [directory] ❯
 *   RIGHT: 0.043s ✔ / 0.043s ✘
 *
 * Usage:
 *   bxrd_prompt left  <exit_code> <elapsed_secs> <venv_path> <cwd>
 *   bxrd_prompt right <exit_code> <elapsed_secs>
 *
 * Build:
 *   gcc -O2 -Wall -Wextra -Wpedantic -std=c11 \
 *       -fstack-protector-strong -D_FORTIFY_SOURCE=2 \
 *       -fPIE -pie -o bxrd_prompt bxrd_prompt.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── ANSI escape sequences ────────────────────────────────────────────────── */
/* %{ %} tell zsh these bytes are zero-width (don't count toward line length). */
/* We use fputs() for all static strings so gcc never sees %{ as a printf      */
/* format specifier.                                                            */

#define ESC_BG256(n)     "%{\033[48;5;" n "m%}"
#define ESC_FG256(n)     "%{\033[38;5;" n "m%}"
#define ESC_BG_TC(r,g,b) "%{\033[48;2;" r ";" g ";" b "m%}"
#define ESC_FG_TC(r,g,b) "%{\033[38;2;" r ";" g ";" b "m%}"
#define ESC_RESET        "%{\033[0m%}"

/* ── Palette ──────────────────────────────────────────────────────────────── */
#define C_VENV_BG    ESC_BG256("52")
#define C_VENV_SEPFG ESC_FG256("52")
#define C_DIR_BG     ESC_BG_TC("20","50","20")
#define C_DIR_SEPFG  ESC_FG_TC("20","50","20")
#define C_WHITE      ESC_FG256("15")
#define C_GREEN      ESC_FG256("2")
#define C_RED        ESC_FG256("1")
#define C_GREY       ESC_FG256("244")
#define C_RESET      ESC_RESET

/* ── Nerd Font glyphs (UTF-8) ─────────────────────────────────────────────── */
#define G_SEP    "\xee\x82\xb0"   /* U+E0B0  solid powerline arrow  */
#define G_FOLDER "\xef\x81\xbb"   /* U+F007B nerd font folder       */
#define G_OK     "\xe2\x9c\x94"   /* U+2714  ✔                      */
#define G_FAIL   "\xe2\x9c\x98"   /* U+2718  ✘                      */
#define G_ARROW  "\xe2\x9d\xaf"   /* U+276F  ❯                      */

/* ── Safe basename — no malloc, no libc basename ─────────────────────────── */
static const char *safe_basename(const char *path)
{
    if (!path || path[0] == '\0') return path;
    const char *base = path;
    for (const char *p = path; *p; p++)
        if (p[0] == '/' && p[1] != '\0')
            base = p + 1;
    return base;
}

/* ── Segment printers ─────────────────────────────────────────────────────── */

static void print_venv(const char *venv_path)
{
    if (!venv_path || venv_path[0] == '\0') return;
    const char *name = safe_basename(venv_path);
    fputs(C_VENV_BG C_WHITE " ", stdout);
    fputs(name, stdout);
    fputs(" " C_VENV_SEPFG C_RESET G_SEP, stdout);
}

static void print_dir(const char *cwd)
{
    if (!cwd || cwd[0] == '\0') cwd = "~";
    fputs(C_DIR_BG C_WHITE " " G_FOLDER " ", stdout);
    fputs(cwd, stdout);
    fputs(" " C_DIR_SEPFG C_RESET G_SEP, stdout);
}

static void print_arrow(void)
{
    fputs(" " C_WHITE G_ARROW C_RESET " ", stdout);
}

/* ── Left / Right ─────────────────────────────────────────────────────────── */

static void prompt_left(const char *venv, const char *cwd)
{
    print_venv(venv);
    print_dir(cwd);
    print_arrow();
}

static void prompt_right(int exit_code, double elapsed)
{
    if (elapsed < 0.0)     elapsed = 0.0;
    if (elapsed > 86400.0) elapsed = 86400.0;

    fputs(C_GREY, stdout);
    printf("%.3fs", elapsed);
    fputs(C_RESET " ", stdout);

    if (exit_code == 0)
        fputs(C_GREEN G_OK C_RESET, stdout);
    else
        fputs(C_RED G_FAIL C_RESET, stdout);
}

/* ── Entry point ──────────────────────────────────────────────────────────── */
int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr,
            "usage: bxrd_prompt left  <exit_code> <elapsed> <venv> <cwd>\n"
            "       bxrd_prompt right <exit_code> <elapsed>\n");
        return 1;
    }

    if (strcmp(argv[1], "left") == 0) {
        if (argc < 6) {
            fprintf(stderr, "bxrd_prompt left: needs exit_code elapsed venv cwd\n");
            return 1;
        }
        prompt_left(argv[4], argv[5]);

    } else if (strcmp(argv[1], "right") == 0) {
        if (argc < 4) {
            fprintf(stderr, "bxrd_prompt right: needs exit_code elapsed\n");
            return 1;
        }
        prompt_right(atoi(argv[2]), atof(argv[3]));

    } else {
        fprintf(stderr, "bxrd_prompt: unknown side '%s'\n", argv[1]);
        return 1;
    }

    return 0;
}
