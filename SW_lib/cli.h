/*----------------------------------------------------------------------------
 * Author: Duy Vinh To
 *
 * Created date: 25th July 2016
 *
 * Describe: Command Line Interface. Header file.
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * INCLUDE
 -----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
 * TYPEDEFS
 -----------------------------------------------------------------------------*/
typedef unsigned char U8;
typedef char S8;
typedef unsigned short U16;
typedef unsigned long U32;

typedef void (*CLI_cmdHdl_CB)(U8 *input, U16 outputsize, U8 *output);
typedef void (*CLI_printCB)(const U8 *s,...);

/*----------------------------------------------------------------------------
 * FUNC.PROTOTYPES
 -----------------------------------------------------------------------------*/
extern void CLI_setup(void);
extern U8 CLI_regCmdHdl(const U8 *cmd, CLI_cmdHdl_CB cmdHdlCB);
extern void CLI_processCmd(U8 *cmd, U16 outputsize, U8 *output);
extern void CLI_show(CLI_printCB printCB, const U8 *printlayout);
