/*
 *     AAA    M M    OOO    SSSS
 *    A   A  M M M  O   O  S 
 *    AAAAA  M M M  O   O   SSS
 *    A   A  M   M  O   O      S
 *    A   A  M   M   OOO   SSSS 
 *
 *    Author:  Stephen Fewer
 *    Contact: steve [AT] harmonysecurity [DOT] com
 *    Web:     http://amos.harmonysecurity.com/
 *    License: GNU General Public License (GPL)
 */

#include <kernel/pm/elf.h>
#include <kernel/fs/vfs.h>

int elf_load( struct VFS_HANDLE * handle )
{
	struct ELF_HDR hdr;
	struct ELF_SECTION_HDR section_hdr;
	int i;
	
	// read in the file header
	if( vfs_read( handle, (void *)&hdr, sizeof(struct ELF_HDR) ) == VFS_FAIL )
		return -1;
	
	// make sure we are dealing with an ELF file
	if( hdr.e_ident[0] != 0x7F && hdr.e_ident[1] != 'E' && hdr.e_ident[2] != 'L' && hdr.e_ident[3] != 'F' )
		return -2;
	
	// 32-bit object
	if( hdr.e_ident[4] != ELF_CLASS_32 )
		return -3;

	//kernel_printf("e_entry: %x\n", hdr.e_entry );
	//kernel_printf("e_shoff: %d\n", hdr.e_shoff );
	//kernel_printf("e_shnum: %d\n", hdr.e_shnum );
	//kernel_printf("e_shentsize: %d\n", hdr.e_shentsize );
	//kernel_printf("sizeof(struct ELF_SECTION_HDR): %d\n", sizeof(struct ELF_SECTION_HDR) );
	
	vfs_seek( handle, hdr.e_shoff, VFS_SEEK_START );
	for( i=0 ; i<hdr.e_shnum ; i++)
	{
		if( vfs_read( handle, (void *)&section_hdr, hdr.e_phentsize ) == VFS_FAIL )
			return -4-i;
		
		kernel_printf("[%d] name index: %d addr: %x\n", i, section_hdr.sh_name, section_hdr.sh_addr );
	}
	
	return 0;
}
