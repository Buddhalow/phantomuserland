#define DEBUG_MSG_PREFIX "boot"
#include "debug_ext.h"
static int debug_level_flow = 1;
static int debug_level_error = 1;

#include <phantom_types.h>
#include <phantom_assert.h>
#include <phantom_libc.h>
#include <string.h>
#include <multiboot.h>
#include <kernel/vm.h>
#include <kernel/boot.h>
//#include <hal.h>
#include "misc.h"

//#define DEBUG 1

#define MAXTOK 1024

int main_argc;
const char **main_argv;
int main_envc;
const char **main_env;

int boot_argc;
const char **boot_argv;


static const char *default_argv[] = {"phantom", 0};
static const char *default_env[] = {0};


/**
 *
 * Mut\ltiboot cmd line comes like:
 *   kernel_name [<booting-options and foo=bar> --] <args to main>
 *
 * Result:
 *
 *      boot_argv will contain booting-options
 *      main_argv will contain args to main
 *      main_env will contain foo=bar
 *
**/

void
phantom_parse_cmd_line()
{
    int i;

    // Put defaults first

    boot_argc = 0;
    boot_argv = main_argv + 1; // Just 0

    main_argv = default_argv;
    main_argc = 1;
    main_env = default_env;

    if( ! (bootParameters.flags & MULTIBOOT_CMDLINE) )
    {
        return;
    }

    const char* cmdline = (const char*)phystokv(bootParameters.cmdline);

    SHOW_FLOW( 2, "Cmdline: '%s'", cmdline );


    // Break cmd line into the tokens

    const char *token[MAXTOK];
    const char *token_start[MAXTOK];
    int 	token_len[MAXTOK];
    int 	ntoken = 0;

    {
        const char *cp = cmdline;

        // Skip wspace
        while( *cp && isspace( *cp ))
            cp++;

        const char *last_token_start = cp;
        token_start[ntoken] = cp;
        ntoken++;

        while(*cp)
        {
            // Skip !wspace
            while(*cp && !isspace(*cp))
                cp++;

            if( last_token_start != 0 )
            {
                assert(ntoken > 0 );
                token_len[ntoken-1] = cp - last_token_start;
                token[ntoken-1] = strndup( token_start[ntoken-1], token_len[ntoken-1] );
            }

            if( ntoken >= MAXTOK )
            {
                printf("too many tokens on cmdline");
                break;
            }

            // Skip wspace
            while( *cp && isspace( *cp ))
                cp++;

            token_start[ntoken] = last_token_start = cp;
            ntoken++;


        }

        if( last_token_start != 0 )
        {
            assert(ntoken > 0 );
            token_len[ntoken-1] = cp - last_token_start;
            token[ntoken-1] = strndup( token_start[ntoken-1], token_len[ntoken-1] );
        }

    }

    // Kill empty token at end.
    while( ntoken > 0 && token_len[ntoken-1] == 0)
        ntoken--;


    // Hack. Kernel name is not guaranteed to be on cmd line.
    // Try to find it out by checking first token for - or = chars
    int start_opt = 1; // Assume 0 is kernel name

    if( token[0][0] == '-' ||
        strchr( token[0], '=' ) != 0 )
        start_opt = 0; // No

    int end_boot_opts = -1;

    for( i = 0; i < ntoken; i++ )
    {
        SHOW_FLOW( 2, "'%s'", token[i]);

        if( 0 == strcmp( token[i], "--" ) )
        {
            end_boot_opts = i;
            SHOW_FLOW0( 2, " EBOOT");

        }
    }

    // + 4 for final zeros
    const char **vector = malloc( sizeof(void *) * (ntoken + 4 ));

    // Have env and/or boot opts?
    if(end_boot_opts >= 0)
    {
        // go for boot opts
        boot_argv = vector;
        boot_argc = 0;
        // start from start_opt, skip kernel name, if there
        for( i = start_opt; i < end_boot_opts; i++ )
        {
            if( token[i][0] == '-' )
            {
                *vector++ = token[i];
                boot_argc++;
            }
        }
        *vector++ = 0;

        // go for env now
        main_env = vector;
        main_envc = 0;
        // start from start_opt, skip kernel name, if there
        for( i = start_opt; i < end_boot_opts; i++ )
        {

            const char *t = token[i];
            if( *t != '-' )
            {
                *vector++ = t;
                main_envc++;
            }
        }
        *vector++ = 0;

    }



    // go for main opts now
    main_argv = vector;
    main_argc = 0;

    if(start_opt)
        *vector++ = token[0]; // kernel name
    else
        *vector++ = default_argv[0]; // No kernel name, use default
    main_argc++;


    i = start_opt;
    if(end_boot_opts >= 0)
        i = end_boot_opts + 1;
    for( ; i < ntoken; i++ )
    {
        *vector++ = token[i];
        main_argc++;
    }
    *vector++ = 0;

    SHOW_FLOW0( 1, "Boot argv:" );
    for( i = 0; i < boot_argc; i++ )
        SHOW_FLOW( 1, "\t%s", boot_argv[i] );

    SHOW_FLOW0( 1, "Main argv:" );
    for( i = 0; i < main_argc; i++ )
        SHOW_FLOW( 1, "\t%s", main_argv[i] );

    SHOW_FLOW0( 1, "Main env:" );
    for( i = 0; i < main_envc; i++ )
        SHOW_FLOW( 1, "\t%s", main_env[i] );

}

void
phantom_process_boot_options(void)
{
    int c = boot_argc;
    const char **args = boot_argv;

    while(c--)
    {
        const char *arg = *args++;
        SHOW_ERROR( 0, "Warning: Unknownm boot option '%s'", arg);
    }

}
