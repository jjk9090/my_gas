#define COMMON
#include "as.h"
#include "subsegs.h"
#include "sb.h"
#include "macro.h"
#include "bfdver.h"
#include "output-file.h"
#include <stdio.h>

#ifdef DEFAULT_FLAG_COMPRESS_DEBUG
enum compressed_debug_section_type flag_compress_debug
  = DEFAULT_COMPRESSED_DEBUG_ALGORITHM;
#define DEFAULT_COMPRESSED_DEBUG_ALGORITHM_HELP \
        DEFAULT_COMPRESSED_DEBUG_ALGORITHM
#else
#define DEFAULT_COMPRESSED_DEBUG_ALGORITHM_HELP COMPRESS_DEBUG_NONE
#endif
extern const bfd_target *const *const bfd_target_vector;
/* 开启详细模式 */
int verbose = 0;
/* 默认的阻塞块大小 */
int chunksize = 0;
/* argv[0]  */
static char * myname;
int listing;

// 标准节
segT reg_section;
segT expr_section;
segT text_section;
segT data_section;
segT bss_section;
extern void pobegin(void);

void
print_version_id (void) {
    static int printed;

    if (printed)
        return;
    printed = 1;

    fprintf (stderr, _("GNU assembler version %s (%s) using BFD version %s\n"),
        VERSION, TARGET_ALIAS, BFD_VERSION_STRING);
}

// 通用选项
static void
show_usage (FILE * stream) {
    fprintf (stream, _("Usage: %s [option...] [asmfile...]\n"), myname);

    fprintf (stream, _("\
    Options:\n\
    -a[sub-option...]	  turn on listings\n\
                            Sub-options [default hls]:\n\
                            c      omit false conditionals\n\
                            d      omit debugging directives\n\
                            g      include general info\n\
                            h      include high-level source\n\
                            l      include assembly\n\
                            m      include macro expansions\n\
                            n      omit forms processing\n\
                            s      include symbols\n\
                            =FILE  list to FILE (must be last sub-option)\n"));

    fprintf (stream, _("\
    --alternate             initially turn on alternate macro syntax\n"));
    fprintf (stream, _("\
    --compress-debug-sections[={none|zlib|zlib-gnu|zlib-gabi|zstd}]\n\
                            compress DWARF debug sections\n")),
    //   fprintf (stream, _("\
    // 		            Default: %s\n"),
    // 	   bfd_get_compression_algorithm_name
    //              (DEFAULT_COMPRESSED_DEBUG_ALGORITHM_HELP));
    fprintf (stream, _("\
    --nocompress-debug-sections\n\
                            don't compress DWARF debug sections\n"));
    fprintf (stream, _("\
    -D                      produce assembler debugging messages\n"));
    fprintf (stream, _("\
    --dump-config           display how the assembler is configured and then exit\n"));
    fprintf (stream, _("\
    --debug-prefix-map OLD=NEW\n\
                            map OLD to NEW in debug information\n"));
    fprintf (stream, _("\
    --defsym SYM=VAL        define symbol SYM to given value\n"));
#ifdef USE_EMULATIONS
    {
        int i;
        const char *def_em;

        fprintf (stream, "\
    --emulation=[");
        for (i = 0; i < n_emulations - 1; i++)
        fprintf (stream, "%s | ", emulations[i]->name);
        fprintf (stream, "%s]\n", emulations[i]->name);

        def_em = getenv (EMULATION_ENVIRON);
        if (!def_em)
        def_em = DEFAULT_EMULATION;
        fprintf (stream, _("\
                            emulate output (default %s)\n"), def_em);
    }
#endif
#if defined OBJ_ELF || defined OBJ_MAYBE_ELF
    fprintf (stream, _("\
    --execstack             require executable stack for this object\n"));
    fprintf (stream, _("\
    --noexecstack           don't require executable stack for this object\n"));
    fprintf (stream, _("\
    --size-check=[error|warning]\n\
                ELF .size directive check (default --size-check=error)\n"));
    fprintf (stream, _("\
    --elf-stt-common=[no|yes] "));
    if (DEFAULT_GENERATE_ELF_STT_COMMON)
        fprintf (stream, _("(default: yes)\n"));
    else
        fprintf (stream, _("(default: no)\n"));
    fprintf (stream, _("\
                            generate ELF common symbols with STT_COMMON type\n"));
    fprintf (stream, _("\
    --sectname-subst        enable section name substitution sequences\n"));

    fprintf (stream, _("\
    --generate-missing-build-notes=[no|yes] "));
#if DEFAULT_GENERATE_BUILD_NOTES
    fprintf (stream, _("(default: yes)\n"));
#else
    fprintf (stream, _("(default: no)\n"));
#endif
    fprintf (stream, _("\
                            generate GNU Build notes if none are present in the input\n"));
    fprintf (stream, _("\
    --gsframe               generate SFrame stack trace information\n"));
#endif /* OBJ_ELF */

    fprintf (stream, _("\
    -f                      skip whitespace and comment preprocessing\n"));
    fprintf (stream, _("\
    -g --gen-debug          generate debugging information\n"));
    fprintf (stream, _("\
    --gstabs                generate STABS debugging information\n"));
    fprintf (stream, _("\
    --gstabs+               generate STABS debug info with GNU extensions\n"));
    fprintf (stream, _("\
    --gdwarf-<N>            generate DWARF<N> debugging information. 2 <= <N> <= 5\n"));
    fprintf (stream, _("\
    --gdwarf-cie-version=<N> generate version 1, 3 or 4 DWARF CIEs\n"));
    fprintf (stream, _("\
    --gdwarf-sections       generate per-function section names for DWARF line information\n"));
#if defined (TE_PE) && defined (O_secrel)
    fprintf (stream, _("\
    --gcodeview             generate CodeView debugging information\n"));
#endif
    fprintf (stream, _("\
    --hash-size=<N>         ignored\n"));
    fprintf (stream, _("\
    --help                  show all assembler options\n"));
    fprintf (stream, _("\
    --target-help           show target specific options\n"));
    fprintf (stream, _("\
    -I DIR                  add DIR to search list for .include directives\n"));
    fprintf (stream, _("\
    -J                      don't warn about signed overflow\n"));
    fprintf (stream, _("\
    -K                      warn when differences altered for long displacements\n"));
    fprintf (stream, _("\
    -L,--keep-locals        keep local symbols (e.g. starting with `L')\n"));
    fprintf (stream, _("\
    -M,--mri                assemble in MRI compatibility mode\n"));
    fprintf (stream, _("\
    --MD FILE               write dependency information in FILE (default none)\n"));
    fprintf (stream, _("\
    --multibyte-handling=<method>\n\
                            what to do with multibyte characters encountered in the input\n"));
    fprintf (stream, _("\
    -nocpp                  ignored\n"));
    fprintf (stream, _("\
    -no-pad-sections        do not pad the end of sections to alignment boundaries\n"));
    fprintf (stream, _("\
    -o OBJFILE              name the object-file output OBJFILE (default a.out)\n"));
    fprintf (stream, _("\
    -R                      fold data section into text section\n"));
    fprintf (stream, _("\
    --reduce-memory-overheads ignored\n"));
    fprintf (stream, _("\
    --statistics            print various measured statistics from execution\n"));
    fprintf (stream, _("\
    --strip-local-absolute  strip local absolute symbols\n"));
    fprintf (stream, _("\
    --traditional-format    Use same format as native assembler when possible\n"));
    fprintf (stream, _("\
    --version               print assembler version number and exit\n"));
    fprintf (stream, _("\
    -W  --no-warn           suppress warnings\n"));
    fprintf (stream, _("\
    --warn                  don't suppress warnings\n"));
    fprintf (stream, _("\
    --fatal-warnings        treat warnings as errors\n"));
#ifdef HAVE_ITBL_CPU
    fprintf (stream, _("\
    --itbl INSTTBL          extend instruction set to include instructions\n\
                          matching the specifications defined in file INSTTBL\n"));
#endif
    fprintf (stream, _("\
    -w                      ignored\n"));
    fprintf (stream, _("\
    -X                      ignored\n"));
    fprintf (stream, _("\
    -Z                      generate object file even after errors\n"));
    fprintf (stream, _("\
    --listing-lhs-width     set the width in words of the output data column of\n\
                            the listing\n"));
    fprintf (stream, _("\
    --listing-lhs-width2    set the width in words of the continuation lines\n\
                            of the output data column; ignored if smaller than\n\
                            the width of the first line\n"));
    fprintf (stream, _("\
    --listing-rhs-width     set the max width in characters of the lines from\n\
                            the source file\n"));
    fprintf (stream, _("\
    --listing-cont-lines    set the maximum number of continuation lines used\n\
                            for the output data column of the listing\n"));
    fprintf (stream, _("\
    @FILE                   read options from FILE\n"));
    

    md_show_usage (stream);

    fputc ('\n', stream);

    if (REPORT_BUGS_TO[0] && stream == stdout)
        fprintf (stream, _("Report bugs to %s\n"), REPORT_BUGS_TO);
}

static void
parse_args (int * pargc, char *** pargv) {
    int old_argc;
    int new_argc;
    char ** old_argv;
    char ** new_argv;
    char *shortopts;
    extern const char *md_shortopts;
    // -o
    static const char std_shortopts[] = {
        '-', 'J',
    #ifndef WORKING_DOT_WORD
        'K',
    #endif
        'L', 'M', 'R', 'W', 'Z', 'a', ':', ':', 'D', 'f', 'g', ':',':', 'I', ':', 'o', ':',
    #ifndef VMS
        /* -v takes an argument on VMS, so we don't make it a generic
           option.  */
        'v',
    #endif
        'w', 'X',
    #ifdef HAVE_ITBL_CPU
        /* New option for extending instruction set (see also --itbl below).  */
    't', ':',
    #endif
        '\0'
    };
    // --host
    struct option *longopts;
    extern struct option md_longopts[];
    extern size_t md_longopts_size;
    enum option_values
    {
      OPTION_HELP = OPTION_STD_BASE,
      OPTION_NOCPP,
      OPTION_STATISTICS,
      OPTION_VERSION,
      OPTION_DUMPCONFIG,
      OPTION_VERBOSE,
      OPTION_EMULATION,
      OPTION_DEBUG_PREFIX_MAP,
      OPTION_DEFSYM,
      OPTION_LISTING_LHS_WIDTH,
      OPTION_LISTING_LHS_WIDTH2, /* = STD_BASE + 10 */
      OPTION_LISTING_RHS_WIDTH,
      OPTION_LISTING_CONT_LINES,
      OPTION_DEPFILE,
      OPTION_GSTABS,
      OPTION_GSTABS_PLUS,
      OPTION_GDWARF_2,
      OPTION_GDWARF_3,
      OPTION_GDWARF_4,
      OPTION_GDWARF_5,
      OPTION_GDWARF_SECTIONS, /* = STD_BASE + 20 */
      OPTION_GDWARF_CIE_VERSION,
      OPTION_GCODEVIEW,
      OPTION_STRIP_LOCAL_ABSOLUTE,
      OPTION_TRADITIONAL_FORMAT,
      OPTION_WARN,
      OPTION_TARGET_HELP,
      OPTION_EXECSTACK,
      OPTION_NOEXECSTACK,
      OPTION_SIZE_CHECK,
      OPTION_ELF_STT_COMMON,
      OPTION_ELF_BUILD_NOTES, /* = STD_BASE + 30 */
      OPTION_SECTNAME_SUBST,
      OPTION_ALTERNATE,
      OPTION_AL,
      OPTION_HASH_TABLE_SIZE,
      OPTION_REDUCE_MEMORY_OVERHEADS,
      OPTION_WARN_FATAL,
      OPTION_COMPRESS_DEBUG,
      OPTION_NOCOMPRESS_DEBUG,
      OPTION_NO_PAD_SECTIONS,
      OPTION_MULTIBYTE_HANDLING,  /* = STD_BASE + 40 */
      OPTION_SFRAME
    /* When you add options here, check that they do
       not collide with OPTION_MD_BASE.  See as.h.  */
    };
    static const struct option std_longopts[] =
  {
    /* Note: commas are placed at the start of the line rather than
       the end of the preceding line so that it is simpler to
       selectively add and remove lines from this list.  */
    {"alternate", no_argument, NULL, OPTION_ALTERNATE}
    /* The entry for "a" is here to prevent getopt_long_only() from
       considering that -a is an abbreviation for --alternate.  This is
       necessary because -a=<FILE> is a valid switch but getopt would
       normally reject it since --alternate does not take an argument.  */
    ,{"a", optional_argument, NULL, 'a'}
    /* Handle -al=<FILE>.  */
    ,{"al", optional_argument, NULL, OPTION_AL}
    ,{"compress-debug-sections", optional_argument, NULL, OPTION_COMPRESS_DEBUG}
    ,{"nocompress-debug-sections", no_argument, NULL, OPTION_NOCOMPRESS_DEBUG}
    ,{"debug-prefix-map", required_argument, NULL, OPTION_DEBUG_PREFIX_MAP}
    ,{"defsym", required_argument, NULL, OPTION_DEFSYM}
    ,{"dump-config", no_argument, NULL, OPTION_DUMPCONFIG}
    ,{"emulation", required_argument, NULL, OPTION_EMULATION}
#if defined OBJ_ELF || defined OBJ_MAYBE_ELF
    ,{"execstack", no_argument, NULL, OPTION_EXECSTACK}
    ,{"noexecstack", no_argument, NULL, OPTION_NOEXECSTACK}
    ,{"size-check", required_argument, NULL, OPTION_SIZE_CHECK}
    ,{"elf-stt-common", required_argument, NULL, OPTION_ELF_STT_COMMON}
    ,{"sectname-subst", no_argument, NULL, OPTION_SECTNAME_SUBST}
    ,{"generate-missing-build-notes", required_argument, NULL, OPTION_ELF_BUILD_NOTES}
    ,{"gsframe", no_argument, NULL, OPTION_SFRAME}
#endif
    ,{"fatal-warnings", no_argument, NULL, OPTION_WARN_FATAL}
    ,{"gdwarf-2", no_argument, NULL, OPTION_GDWARF_2}
    ,{"gdwarf-3", no_argument, NULL, OPTION_GDWARF_3}
    ,{"gdwarf-4", no_argument, NULL, OPTION_GDWARF_4}
    ,{"gdwarf-5", no_argument, NULL, OPTION_GDWARF_5}
    /* GCC uses --gdwarf-2 but GAS used to to use --gdwarf2,
       so we keep it here for backwards compatibility.  */
    ,{"gdwarf2", no_argument, NULL, OPTION_GDWARF_2}
    ,{"gdwarf-sections", no_argument, NULL, OPTION_GDWARF_SECTIONS}
    ,{"gdwarf-cie-version", required_argument, NULL, OPTION_GDWARF_CIE_VERSION}
#if defined (TE_PE) && defined (O_secrel)
    ,{"gcodeview", no_argument, NULL, OPTION_GCODEVIEW}
#endif
    ,{"gen-debug", no_argument, NULL, 'g'}
    ,{"gstabs", no_argument, NULL, OPTION_GSTABS}
    ,{"gstabs+", no_argument, NULL, OPTION_GSTABS_PLUS}
    ,{"hash-size", required_argument, NULL, OPTION_HASH_TABLE_SIZE}
    ,{"help", no_argument, NULL, OPTION_HELP}
#ifdef HAVE_ITBL_CPU
    /* New option for extending instruction set (see also -t above).
       The "-t file" or "--itbl file" option extends the basic set of
       valid instructions by reading "file", a text file containing a
       list of instruction formats.  The additional opcodes and their
       formats are added to the built-in set of instructions, and
       mnemonics for new registers may also be defined.  */
    ,{"itbl", required_argument, NULL, 't'}
#endif
    /* getopt allows abbreviations, so we do this to stop it from
       treating -k as an abbreviation for --keep-locals.  Some
       ports use -k to enable PIC assembly.  */
    ,{"keep-locals", no_argument, NULL, 'L'}
    ,{"keep-locals", no_argument, NULL, 'L'}
    ,{"listing-lhs-width", required_argument, NULL, OPTION_LISTING_LHS_WIDTH}
    ,{"listing-lhs-width2", required_argument, NULL, OPTION_LISTING_LHS_WIDTH2}
    ,{"listing-rhs-width", required_argument, NULL, OPTION_LISTING_RHS_WIDTH}
    ,{"listing-cont-lines", required_argument, NULL, OPTION_LISTING_CONT_LINES}
    ,{"MD", required_argument, NULL, OPTION_DEPFILE}
    ,{"mri", no_argument, NULL, 'M'}
    ,{"nocpp", no_argument, NULL, OPTION_NOCPP}
    ,{"no-pad-sections", no_argument, NULL, OPTION_NO_PAD_SECTIONS}
    ,{"no-warn", no_argument, NULL, 'W'}
    ,{"reduce-memory-overheads", no_argument, NULL, OPTION_REDUCE_MEMORY_OVERHEADS}
    ,{"statistics", no_argument, NULL, OPTION_STATISTICS}
    ,{"strip-local-absolute", no_argument, NULL, OPTION_STRIP_LOCAL_ABSOLUTE}
    ,{"version", no_argument, NULL, OPTION_VERSION}
    ,{"verbose", no_argument, NULL, OPTION_VERBOSE}
    ,{"target-help", no_argument, NULL, OPTION_TARGET_HELP}
    ,{"traditional-format", no_argument, NULL, OPTION_TRADITIONAL_FORMAT}
    ,{"warn", no_argument, NULL, OPTION_WARN}
    ,{"multibyte-handling", required_argument, NULL, OPTION_MULTIBYTE_HANDLING}
  };
    shortopts = concat (std_shortopts, md_shortopts, (char *) NULL);
    longopts = (struct option *) xmalloc (sizeof (std_longopts)
                                        + md_longopts_size + sizeof (struct option));
    // std_longopts 数组的内容复制到 longopts 数组中
    memcpy (longopts, std_longopts, sizeof (std_longopts));
    // 将 md_longopts 数组的内容复制到 longopts 数组的末尾
    memcpy (((char *) longopts) + sizeof (std_longopts), md_longopts, md_longopts_size);
    memset (((char *) longopts) + sizeof (std_longopts) + md_longopts_size,
        0, sizeof (struct option));
    //  复制旧的命令行参数
    old_argc = *pargc;
    old_argv = *pargv;
    // 初始化新的argv
    new_argv = notes_alloc (sizeof (char *) * (old_argc + 1));
    new_argv[0] = old_argv[0];
    new_argc = 1;
    new_argv[new_argc] = NULL;
    while (1) {
        // 允许长选项不仅可以以 -- 开头，还可以以 - 开头来指示长选项
        int longind;
        int optc = getopt_long_only (old_argc, old_argv, shortopts, longopts,
				   &longind);
        if (optc == -1) break;
        switch (optc) {
            default:
                // 识别optc,返回1
                if (md_parse_option (optc, optarg) != 0)
	                break;
                if (optc == 'v') {
                    case 'v':
                    case OPTION_VERBOSE:
                        print_version_id ();
                        verbose = 1;
                        break;
                } else {
                    // as_bad (_("unrecognized option -%c%s"), optc, optarg ? optarg : "");
                }
            case '?':
	            exit (EXIT_FAILURE);
            case 1:
                // 是文件名
                if (!strcmp (optarg, "-"))
                	optarg = (char *) "";
                new_argv[new_argc++] = optarg;   
                new_argv[new_argc] = NULL;
                break;
            case OPTION_TARGET_HELP:
                // 特定目标
                md_show_usage (stdout);
                exit (EXIT_SUCCESS);
            case OPTION_HELP:
                // 通用目标
                show_usage (stdout);
                exit (EXIT_SUCCESS);
            case OPTION_NOCPP:
                break;
            case 'o':
                out_file_name = notes_strdup (optarg);
                break;
        }      
    }
    free (shortopts);
    free (longopts);

    *pargc = new_argc;
    *pargv = new_argv;
    printf("parse over\n");
}

static void
free_notes (void) {
    _obstack_free (&notes, NULL);
}

static void
gas_early_init (int *argcp, char ***argvp) {
#ifndef OBJ_DEFAULT_OUTPUT_FILE_NAME
#define OBJ_DEFAULT_OUTPUT_FILE_NAME "a.out"
#endif

    out_file_name = OBJ_DEFAULT_OUTPUT_FILE_NAME;
    if (bfd_init () != BFD_INIT_MAGIC)
        as_fatal (_("libbfd ABI mismatch"));
    // 初始化obstack 靠
    obstack_begin (&notes, chunksize);
    xatexit (free_notes);

    myname = **argvp;
    xmalloc_set_program_name (myname);
    bfd_set_error_program_name (myname);
}

static void
gas_init (void) {
    // 初始化 symbol
    symbol_begin ();
    frag_init ();
    subsegs_begin ();
    read_begin();
    input_scrub_begin();
    expr_begin ();
    eh_begin ();

    // 宏初始化
    macro_init ();
    /* 标识版本的符号 */
    local_symbol_make (".gasversion.", absolute_section,
        &predefined_address_frag, BFD_VERSION / 10000UL);
    xatexit (output_file_close);
    // 创建输出文件
    output_file_create (out_file_name);
    gas_assert (stdoutput != 0);
    // 初始化点符号
    dot_symbol_init ();
}

/**
 * @Description: 扫描文件
 */
static void
perform_an_assembly_pass (int argc, char ** argv) {
    int saw_a_file = 0;
#ifndef OBJ_MACH_O
    flagword applicable;
#endif
    need_pass_2 = 0;
#ifndef OBJ_MACH_O
    // 创建标准的节（section）以及汇编器内部使用的节
    text_section = subseg_new (TEXT_SECTION_NAME, 0);
    data_section = subseg_new (DATA_SECTION_NAME, 0);
    bss_section = subseg_new (BSS_SECTION_NAME, 0);

    applicable = bfd_applicable_section_flags (stdoutput);
    bfd_set_section_flags (text_section,
			 applicable & (SEC_ALLOC | SEC_LOAD | SEC_RELOC
				       | SEC_CODE | SEC_READONLY));
    bfd_set_section_flags (data_section,
                applicable & (SEC_ALLOC | SEC_LOAD | SEC_RELOC
                        | SEC_DATA));
    bfd_set_section_flags (bss_section, applicable & SEC_ALLOC);
    seg_info (bss_section)->bss = 1;
#endif
    // 绝对段和未知段
    subseg_new (BFD_ABS_SECTION_NAME, 0);
    subseg_new (BFD_UND_SECTION_NAME, 0);
    reg_section = subseg_new ("*GAS `reg' section*", 0);
    expr_section = subseg_new ("*GAS `expr' section*", 0);
#ifndef OBJ_MACH_O
    subseg_set (text_section, 0);
#endif
    /* 一些初始化 增加符号表项 */
    md_begin ();
#ifdef obj_begin
    obj_begin ();
#endif
    /* Skip argv[0].  */
    argv++;
    argc--;
    while (argc--) {
        if (*argv) {
            saw_a_file++;
	        read_a_source_file (*argv);
        }
        argv++;	
    }
    if (!saw_a_file)
        read_a_source_file ("");
}

int main(int argc, char **argv) {
    char ** argv_orig = argv;
    struct stat sob;
    // 初始化 md
    gas_early_init (&argc, &argv);

    // 识别参数
    parse_args (&argc, &argv);
    if (argc > 1 && stat (out_file_name,&sob) == 0) {
        int i;
        for (i = 1;i < argc;i++) {
            struct stat sib;
            // 检查输入文件和输出文件是否不一致
            if (stat (argv[i], &sib) == 0
                    && sib.st_ino == sob.st_ino
                    && sib.st_ino != 0
                    && sib.st_dev == sob.st_dev
                    && S_ISREG (sib.st_mode)) {
                const char *saved_out_file_name = out_file_name;
                out_file_name = NULL;
                as_fatal (_("The input '%s' and output '%s' files are the same"),
			        argv[i], saved_out_file_name);
            }
        }
    }
    
    // 初始化符号表之类的
    gas_init ();

    // 读汇编代码 开始汇编
    perform_an_assembly_pass (argc, argv);

    cond_finish_check (-1);
#ifdef md_finish
    md_finish ();
#endif
    keep_it = 0;
    
    if (seen_at_least_1_file ()) {
        int n_warns, n_errs;
        char warn_msg[50];
        char err_msg[50];
        // 写入目标文件
        write_object_file ();
        n_warns = had_warnings ();
        n_errs = had_errors ();

        sprintf (warn_msg,
            ngettext ("%d warning", "%d warnings", n_warns), n_warns);
        sprintf (err_msg,
            ngettext ("%d error", "%d errors", n_errs), n_errs);
        if (n_errs == 0)
            keep_it = 1;
    }
    fflush (stderr);
    input_scrub_end ();

    /* Use xexit instead of return, because under VMS environments they
        may not place the same interpretation on the value given.  */
    if (had_errors () != 0)
        xexit (EXIT_FAILURE);
    /* Only generate dependency file if assembler was successful.  */
    print_dependencies ();
    printf("as over\n");
    xexit (EXIT_SUCCESS);
}
