/* ide.h  ide Registers */

#ifndef _IDE_H_
#define _IDE_H_       

/* IDE I/O mapped registers */
#define DATA	0x00
#define ERROR	0x01
#define FEATURES	0x01		
#define SECTOR_COUNT	0x02
#define LBALO	0x03
#define LBAMID	0x04
#define LBAHI	0x05
#define DRIVEANDHEAD	0x06	
#define COMMAND	0x07
#define STATUS	0x07

/*  Drive & Head */
#define MODE_SLAVE	0x10
#define MODE_LBA	0x40


/* Status Byte */
#define ERR_STAT	0x00
#define INDEX_STAT	0x02
#define ECC_STAT	0x04
#define DRQ_STAT	0x08
#define SEEK_STAT	0x10
#define WERROR_STAT	0x20
#define RDY_STAT	0x40
#define BSY_STAT	0x80


/* Command Byte*/
#define RESET	0x10
#define READ	0x20
#define WRITE	0x30
#define CHKSECTOR	0x40
#define FORMAT	0x50
#define INIT	0x60
#define SEEK	0x70
#define DIAGNOSE	0x90
#define SETUP	0x91

#endif