#ifndef _KERNEL_PM_ELF_H_
#define _KERNEL_PM_ELF_H_

#include <sys/types.h>

#include <kernel/fs/vfs.h>

// See the Portable Formats Specification 1.1 for the documentation on these structures and defines

// The possible values fot the ELF Headers e_type
#define ELF_ET_NONE			0
#define ELF_ET_REL			1
#define ELF_ET_EXEC			2
#define ELF_ET_DYN			3
#define ELF_ET_CORE			4
#define ELF_ET_LOPROC		5
#define ELF_ET_HIPROC		6

// The required architecture for the file, specified in the ELF Headers e_machine
#define ELF_EM_NONE			0
#define ELF_EM_M32			1
#define ELF_EM_SPARC		2
#define ELF_EM_386			3
#define ELF_EM_68K			4
#define ELF_EM_88K			5
#define ELF_EM_486			6
#define ELF_EM_860			7

// The objects file version, specified in the ELF Headers e_version
#define ELF_EV_NONE			0
#define ELF_EV_CURRENT		1

#define	ELF_CLASS_NONE		0
#define	ELF_CLASS_32		1
#define	ELF_CLASS_64		2

#define ELF_EI_NIDENT		16

// Figure 1-3: ELF Header
struct ELF_HDR
{
  BYTE  e_ident[ELF_EI_NIDENT];
  WORD  e_type;
  WORD  e_machine;
  DWORD e_version;
  DWORD e_entry;
  DWORD	e_phoff;
  DWORD	e_shoff;
  DWORD e_flags;
  WORD  e_ehsize;
  WORD  e_phentsize;
  WORD  e_phnum;
  WORD  e_shentsize;
  WORD  e_shnum;
  WORD  e_shstrndx;
};

// Figure 1-9: ELF Section Header
struct ELF_SECTION_HDR
{
	DWORD sh_name;
	DWORD sh_type;
	DWORD sh_flags;
	DWORD sh_addr;
	DWORD sh_offset;
	DWORD sh_size;
	DWORD sh_link;
	DWORD sh_info;
	DWORD sh_addralign;
	DWORD sh_entsize;
};

struct ELF_SECTION_HDR_TABLE
{
    DWORD sht_num;
    DWORD sht_size;
    DWORD sht_addr;
    DWORD sht_shndx;
};

#endif
