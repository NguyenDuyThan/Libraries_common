/*----------------------------------------------------------------------------
 * Author: Duy Vinh To
 *
 * Created date: 25th July 2016
 *
 * Describe: Command Line Interface. Source file.
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * INCLUDE
 -----------------------------------------------------------------------------*/
#include "cli.h"
#include "string.h"
#include "stddef.h"
//#include "stringlib.h"
//#include "dbgPrint.h"

/*----------------------------------------------------------------------------
 * DEFINES
 -----------------------------------------------------------------------------*/
#ifndef CLI_CMDLIST_MAX
#define CLI_CMDLIST_MAX		32 /* This is default value if user hasn't predefine it */
#endif // CLI_CMDLIST_MAX
/*----------------------------------------------------------------------------
 * TYPEDEFS
 -----------------------------------------------------------------------------*/
typedef struct
{
	U8 *cmd;
	CLI_cmdHdl_CB cb;
}CLI_CMD_t;

/*----------------------------------------------------------------------------
 * VARIABLES
 -----------------------------------------------------------------------------*/
static CLI_CMD_t CLI_cmdhdl_list[CLI_CMDLIST_MAX];

/*----------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * INCLUDE
 -----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * brief:
 -----------------------------------------------------------------------------*/
void CLI_setup(void)
{
	memset(CLI_cmdhdl_list, 0, sizeof(CLI_CMD_t) * CLI_CMDLIST_MAX);
}
/*----------------------------------------------------------------------------
 * brief: register new command handler.
 * Param:	cmd			|	IN	|	command body.
 * 			cptype		|	IN	|	command parameter type.
 * 			cmdHdlCB	|	IN	|	command handler callback.
 * Ret:		0	|	OK
 * 			>0	|	FAIL
 -----------------------------------------------------------------------------*/
U8 CLI_regCmdHdl(const U8 *cmd, CLI_cmdHdl_CB cmdHdlCB)
{
	for(U8 i = 0; i < CLI_CMDLIST_MAX; i++)
	{
		if (CLI_cmdhdl_list[i].cmd == NULL)
		{
			CLI_cmdhdl_list[i].cmd = cmd;
			CLI_cmdhdl_list[i].cb = cmdHdlCB;
			return 0;
		}
	}
	return 1;
}
/*----------------------------------------------------------------------------
 * brief:
 -----------------------------------------------------------------------------*/
void CLI_processCmd(U8 *cmd, U16 outputsize, U8 *output)
{
	for(U8 i = 0; i < CLI_CMDLIST_MAX; i++)
	{
		if (CLI_cmdhdl_list[i].cmd != NULL)
		{
			U8 cmdLen = strlen(CLI_cmdhdl_list[i].cmd);
			if (!strncmp(cmd, CLI_cmdhdl_list[i].cmd, cmdLen))
			{
				CLI_cmdhdl_list[i].cb(&cmd[cmdLen], outputsize, output);
				return;
			}
		}
	}
}
/*---------------------------------------------------------------------------------------------
 * Brief: show CLI command list.
 * Param: 	printCB		|	IN	|	print callback function
 * 			printlayout	|	IN	|	print layout. See below example to understand.
 * Note: print layout?
 * print layout is how you want to show each command node.
 * Each command node will have 2 parameters: command and parameter type.
 * All parameters are string (char array).
 * Example:
 * printlayout = "\r\n %s"
 * And result is: "\r\n CMD DEMO"
 ----------------------------------------------------------------------------------------------*/
void CLI_show(CLI_printCB printCB, const U8 *printlayout)
{
	U8 sParam[10];
	for(U8 i = 0; i < CLI_CMDLIST_MAX; i++)
	{
		if (CLI_cmdhdl_list[i].cmd != NULL)
		{
			printCB(printlayout, CLI_cmdhdl_list[i].cmd);
		}
	}
}
