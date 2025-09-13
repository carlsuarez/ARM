#include <kernel/core/task/elf/dynamic.h>

/*
* FIELDS SET IN SO
    - strtab
    - symtab
    - hash
*/
int8_t parse_pt_dynamic(int8_t fd, Elf32_Dyn *dyns, Elf32_Ehdr *hdr, Elf32_Phdr *phdr, Elf32_Addr elf_mem, Elf32_Addr base_va, struct PCB *task, so_entry_t *so)
{
    printk("parse_pt_dynamic\n");
#define MAX_NEEDED 16
    Elf32_Word needed_offsets[MAX_NEEDED];
    size_t needed_index = 0;

    Elf32_Addr pltgot = 0; // Address associated with the procedure linkage table and/or the global offset table
    Elf32_Addr hash = 0;

    Elf32_Addr strtab = 0;
    Elf32_Word strsz = 0;

    Elf32_Addr symtab = 0;
    Elf32_Word syment = 0;

    Elf32_Addr rela = 0;    // Address of the DT_RELA table
    Elf32_Word relasz = 0;  // Size in bytes of the DT_RELA table
    Elf32_Word relaent = 0; // Size in bytes of DT_RELA entry

    Elf32_Addr rel = 0;    // Address of the DT_REL table
    Elf32_Word relsz = 0;  // Size in bytes of the DT_REL table
    Elf32_Word relent = 0; // Size in bytes of DT_REL entry

    Elf32_Word pltrel = 0; // Type of relocation entry associated with the PLT

    Elf32_Addr jmprel = 0;   // Address of relocation entries associated solely with the procedure linkage table
    Elf32_Word pltrelsz = 0; // Size of the relocation entries associated with the procedure linkage table

    Elf32_Dyn *dyn;
    size_t i = 0;
    do
    {
        dyn = dyns + i;
        switch (dyn->d_tag)
        {
        case DT_NEEDED:
            needed_offsets[needed_index++] = dyn->d_un.d_val;
            break;

        case DT_PLTGOT:
            pltgot = dyn->d_un.d_ptr - base_va;
            break;

        case DT_STRTAB:
            strtab = dyn->d_un.d_ptr - base_va;
            break;
        case DT_STRSZ:
            strsz = dyn->d_un.d_val;
            break;

        case DT_SYMTAB:
            symtab = dyn->d_un.d_ptr - base_va;
            break;
        case DT_SYMENT:
            syment = dyn->d_un.d_val;
            break;

        case DT_HASH:
            hash = dyn->d_un.d_ptr - base_va;
            break;

        case DT_REL:
            rel = dyn->d_un.d_ptr - base_va;
            break;
        case DT_RELSZ:
            relsz = dyn->d_un.d_val;
            break;
        case DT_RELENT:
            relent = dyn->d_un.d_val;
            break;

        case DT_RELA:
            rela = dyn->d_un.d_ptr - base_va;
            break;
        case DT_RELASZ:
            relasz = dyn->d_un.d_val;
            break;
        case DT_RELAENT:
            relaent = dyn->d_un.d_val;
            break;

        case DT_PLTREL:
            pltrel = dyn->d_un.d_val;
            break;

        case DT_PLTRELSZ:
            pltrelsz = dyn->d_un.d_val;
            break;

        case DT_JMPREL:
            jmprel = dyn->d_un.d_ptr - base_va;
            break;

        default:
            break;
        }

        i++;
    } while (dyn->d_tag != DT_NULL);

    // Free the dynamic table
    kfree(dyns);

    // Allocate memory for the string table
    so->strtab = kmalloc(strsz);
    if (!so->strtab)
        return -1;

    printk("strtab: %p\n", so->strtab);

    // Read the string table into memory
    fat32_seek(fd, strtab, SEEK_SET);
    fat32_read(fd, (void *)(so->strtab), strsz);

    // Read the nbucket and nchain of the hash table
    fat32_seek(fd, hash, SEEK_SET);
    fat32_read(fd, &so->hash, sizeof(so->hash.nbucket) + sizeof(so->hash.nchain));

    // Allocate memory for the hash table
    so->hash.bucket = kmalloc(so->hash.nbucket * sizeof(Elf32_Word));
    so->hash.chain = kmalloc(so->hash.nchain * sizeof(Elf32_Word));

    // Read the bucket, then the chain
    fat32_read(fd, so->hash.bucket, so->hash.nbucket * sizeof(Elf32_Word));
    fat32_read(fd, so->hash.chain, so->hash.nchain * sizeof(Elf32_Word));

    if (!so->hash.bucket || !so->hash.chain)
        return -1;

    printk("hash.bucket: %p\n", so->hash.bucket);
    printk("hash.chain: %p\n", so->hash.chain);

    // Allocate memory for symtab
    so->symtab = kmalloc(so->hash.nchain * syment);

    if (!so->symtab)
        return -1;

    printk("symtab: %p\n", so->symtab);

    // Read the symbol table into memory
    fat32_seek(fd, symtab, SEEK_SET);
    fat32_read(fd, (void *)(so->symtab), so->hash.nchain * syment);

    // Load all shared objects
    for (size_t i = 0; i < needed_index; i++)
    {
        char *path = so->strtab + needed_offsets[i];

        // Load the shared object
        load_shared_object(path, task);
    }

    // Apply all relocations
    if (rel && relsz && relent)
    {
        printk("rel && relsz && relent\n");
        apply_relocations(fd, rel, relsz, relent, so->symtab, so->strtab, elf_mem, base_va, task);
    }

    if (rela && relasz && relaent)
    {
        printk("rela && relasz && relaent\n");
        apply_relocations(fd, rela, relasz, relaent, so->symtab, so->strtab, elf_mem, base_va, task);
    }

    // TODO currently, everything is resolved eagerly. To improve efficiency, make lazy resolver
    if (jmprel && pltrelsz && pltrel)
    {
        printk("jmprel && pltrelsz && pltrel\n");
        apply_relocations(fd, jmprel, pltrelsz, pltrel, so->symtab, so->strtab, elf_mem, base_va, task);
    }

#undef MAX_NEEDED

    return 0;
}