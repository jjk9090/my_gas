#define OBJ_HEADER "obj-elf.h"
#include "as.h"
#include"subsegs.h"
static segT previous_section;
static segT comment_section;
static int previous_subsection;

asection *elf_com_section_ptr;

struct section_stack {
    struct section_stack *next;
    segT seg, prev_seg;
    int subseg, prev_subseg;
};
static struct section_stack *section_stack;

static void obj_elf_size (int);
static void obj_elf_type (int);
/*
* 获取节名称
*/
const char *
obj_elf_section_name (void) {
    char *name;
    SKIP_WHITESPACE ();

    if (*input_line_pointer == '"') {

    } else {
        char *end = input_line_pointer;

        while (0 == strchr ("\n\t,; ", *end))
            end++;
        if (end == input_line_pointer) {
            as_bad (_("missing name"));
            ignore_rest_of_line ();
            return NULL;
        }
        obstack_grow0 (&notes, input_line_pointer, end - input_line_pointer);
        name = obstack_base (&notes);

        obstack_finish (&notes);
        input_line_pointer = end;
    }
    SKIP_WHITESPACE ();
    return name;
}

/* ELF section flags for unique sections.  */
#define SEC_ASSEMBLER_SHF_MASK SHF_GNU_RETAIN

static bool
get_section_by_match (bfd *abfd ATTRIBUTE_UNUSED, asection *sec, void *inf) {
    struct elf_section_match *match = (struct elf_section_match *) inf;
    const char *gname = match->group_name;
    const char *group_name = elf_group_name (sec);
    const char *linked_to_symbol_name
        = sec->map_head.linked_to_symbol_name;
    unsigned int sh_info = elf_section_data (sec)->this_hdr.sh_info;
    bfd_vma sh_flags = (elf_section_data (sec)->this_hdr.sh_flags
                & SEC_ASSEMBLER_SHF_MASK);

    return (sh_info == match->sh_info
        && sh_flags == match->sh_flags
        && ((bfd_section_flags (sec) & SEC_ASSEMBLER_SECTION_ID)
            == (match->flags & SEC_ASSEMBLER_SECTION_ID))
        && sec->section_id == match->section_id
        && (group_name == gname
            || (group_name != NULL
            && gname != NULL
            && strcmp (group_name, gname) == 0))
        && (linked_to_symbol_name == match->linked_to_symbol_name
            || (linked_to_symbol_name != NULL
            && match->linked_to_symbol_name != NULL
            && strcmp (linked_to_symbol_name,
                    match->linked_to_symbol_name) == 0)));
}

void
obj_elf_change_section (const char *name,
			unsigned int type,
			bfd_vma attr,
			int entsize,
			struct elf_section_match *match_p,
			int linkonce,
			int push) {
    asection *old_sec;
    segT sec;
    flagword flags;
    const struct elf_backend_data *bed;
    const struct bfd_elf_special_section *ssect;

    if (match_p == NULL) {
        static struct elf_section_match unused_match;
        match_p = &unused_match;
    }

    /* Switch to the section, creating it if necessary.  */
    if (push) {
        struct section_stack *elt;
        elt = XNEW (struct section_stack);
        elt->next = section_stack;
        elt->seg = now_seg;
        elt->prev_seg = previous_section;
        elt->subseg = now_subseg;
        elt->prev_subseg = previous_subsection;
        section_stack = elt;
    }
    obj_elf_section_change_hook ();
    old_sec = bfd_get_section_by_name_if (stdoutput, name, get_section_by_match,
					(void *) match_p);
    if (old_sec) {
        sec = old_sec;
        subseg_set (sec, 0);
    } else
        sec = subseg_force_new (name, 0);
    bed = get_elf_backend_data (stdoutput);
    ssect = (*bed->get_sec_type_attr) (stdoutput, sec);

    if (ssect != NULL) {
        bool override = false;
        if (!override && old_sec == NULL)
            attr |= ssect->attr;
    }

    /* Convert ELF type and flags to BFD flags.  */
    flags = (SEC_RELOC
	   | ((attr & SHF_WRITE) ? 0 : SEC_READONLY)
	   | ((attr & SHF_ALLOC) ? SEC_ALLOC : 0)
	   | (((attr & SHF_ALLOC) && type != SHT_NOBITS) ? SEC_LOAD : 0)
	   | ((attr & SHF_EXECINSTR) ? SEC_CODE : 0)
	   | ((attr & SHF_MERGE) ? SEC_MERGE : 0)
	   | ((attr & SHF_STRINGS) ? SEC_STRINGS : 0)
	   | ((attr & SHF_EXCLUDE) ? SEC_EXCLUDE: 0)
	   | ((attr & SHF_TLS) ? SEC_THREAD_LOCAL : 0));
    
    if (linkonce)
        flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;
    if (old_sec == NULL) {
        symbolS *secsym;

        if (type == SHT_NULL)
            type = bfd_elf_get_default_section_type (flags);
        elf_section_type (sec) = type;
        elf_section_flags (sec) = attr;
        elf_section_data (sec)->this_hdr.sh_info = match_p->sh_info;

        /* Prevent SEC_HAS_CONTENTS from being inadvertently set.  */
        if (type == SHT_NOBITS)
            seg_info (sec)->bss = 1;

        /* Set the section ID and flags.  */
        sec->section_id = match_p->section_id;
        flags |= match_p->flags;

        /* Set the linked-to symbol name.  */
        sec->map_head.linked_to_symbol_name
        = match_p->linked_to_symbol_name;

        bfd_set_section_flags (sec, flags);
        if (flags & SEC_MERGE)
            sec->entsize = entsize;
        elf_group_name (sec) = match_p->group_name;

        /* Add a symbol for this section to the symbol table.  */
        secsym = symbol_find (name);
        if (secsym != NULL) {
            /* We could be repurposing an undefined symbol here: make sure we
                reset sy_value to look like other section symbols in order to avoid
                trying to incorrectly resolve this section symbol later on.  */
            static const expressionS exp = { .X_op = O_constant };
            symbol_set_value_expression (secsym, &exp);
            symbol_set_bfdsym (secsym, sec->symbol);
        } else
            symbol_table_insert (section_symbol (sec));
    }
#ifdef md_elf_section_change_hook
    md_elf_section_change_hook ();
#endif
}
void
obj_elf_section (int push) {
    // 节的名称
    const char *name;
    // 指向节的起始位置
    char *beg;
    // 表示节的类型
    int type, dummy;
    // 节的属性字段
    bfd_vma attr;
    // GNU 特定的节属性字段
    bfd_vma gnu_attr;
    int entsize;
    int linkonce;
    subsegT new_subsection = -1;
    struct elf_section_match match;
    // 链接到的节索引
    unsigned long linked_to_section_index = -1UL;
    name = obj_elf_section_name ();

    if (name == NULL)
        return;

    memset (&match, 0, sizeof (match));

    symbolS * sym;
    if ((sym = symbol_find (name)) != NULL
        && ! symbol_section_p (sym)
        && S_IS_DEFINED (sym)
        && ! S_IS_VOLATILE (sym)
        && ! S_CAN_BE_REDEFINED (sym)) {
        as_bad (_("section name '%s' already defined as another symbol"), name);
        ignore_rest_of_line ();
        return;
    }
    type = SHT_NULL;
    attr = 0;
    gnu_attr = 0;
    entsize = 0;
    linkonce = 0;
done:
    demand_empty_rest_of_line ();

    obj_elf_change_section (name, type, attr, entsize, &match, linkonce,
			  push);
    if (linked_to_section_index != -1UL) {
        elf_section_flags (now_seg) |= SHF_LINK_ORDER;
        elf_section_data (now_seg)->this_hdr.sh_link = linked_to_section_index;
        /* FIXME: Should we perform some sanity checking on the section index ?  */
    }

    if (push && new_subsection != -1)
        subseg_set (now_seg, new_subsection);  
}
void
obj_elf_section_change_hook (void)
{
  // 记录当前段的信息
  previous_section = now_seg;
  previous_subsection = now_subseg;
}
// .data段
void
obj_elf_data (int i) {
    obj_elf_section_change_hook ();
    
    s_data(i);
}

// .bss段
void
obj_elf_bss (int i ATTRIBUTE_UNUSED) {

}
// .text段
void
obj_elf_text (int i) {
    obj_elf_section_change_hook ();

    s_text (i);

}
static const pseudo_typeS elf_pseudo_table[] =
{
  {"comm", obj_elf_common, 0},
  // {"common", obj_elf_common, 1},
  // {"local", obj_elf_local, 0},
  // {"previous", obj_elf_previous, 0},
  {"section", obj_elf_section, 0},
  // {"pushsection", obj_elf_section, 1},
  // {"popsection", obj_elf_popsection, 0},
  {"size", obj_elf_size, 0},
  {"type", obj_elf_type, 0},
  // {"weak", obj_elf_weak, 0},

  /* These define symbol visibility.  */
  // {"internal", obj_elf_visibility, STV_INTERNAL},
  // {"hidden", obj_elf_visibility, STV_HIDDEN},
  // {"protected", obj_elf_visibility, STV_PROTECTED},

  /* These are used for stabs-in-elf configurations.  */
  // {"line", obj_elf_line, 0},


  // /* A GNU extension to change subsection only.  */
  // {"subsection", obj_elf_subsection, 0},

  /* A GNU extension for object attributes.  */
  // {"gnu_attribute", obj_elf_gnu_attribute, 0},

  /* We need to trap the section changing calls to handle .previous.  */
  {"data", obj_elf_data, 0},
  // {"offset", obj_elf_struct, 0},
  {"text", obj_elf_text, 0},
  {"bss", obj_elf_bss, 0},

  /* End sentinel.  */
  {NULL, NULL, 0},
};

#ifndef S_SET_SIZE
#define S_SET_SIZE(S,V) \
  (elf_symbol (symbol_get_bfdsym (S))->internal_elf_sym.st_size = (V))
#endif

void
elf_pop_insert (void)
{
  pop_insert (elf_pseudo_table); 
}

void
elf_obj_symbol_new_hook (symbolS *symbolP)
{
    struct elf_obj_sy *sy_obj;

    sy_obj = symbol_get_obj (symbolP);
    sy_obj->size = NULL;
    sy_obj->versioned_name = NULL;

}

struct group_list
{
  asection **head;		/* Section lists.  */
  unsigned int num_group;	/* Number of lists.  */
  htab_t indexes; /* Maps group name to index in head array.  */
};
static struct group_list groups;

/* This is called when the assembler starts.  */
void
elf_begin (void)
{
    asection *s;

    /* Add symbols for the known sections to the symbol table.  */
    s = bfd_get_section_by_name (stdoutput, TEXT_SECTION_NAME);
    symbol_table_insert (section_symbol (s));
    s = bfd_get_section_by_name (stdoutput, DATA_SECTION_NAME);
    symbol_table_insert (section_symbol (s));
    s = bfd_get_section_by_name (stdoutput, BSS_SECTION_NAME);
    symbol_table_insert (section_symbol (s));
    elf_com_section_ptr = bfd_com_section_ptr;
    previous_section = NULL;
    previous_subsection = 0;
    comment_section = NULL;
    memset (&groups, 0, sizeof (groups));
}

void
elf_frob_file_before_adjust (void) {
  if (symbol_rootP)
    {
      symbolS *symp;

      for (symp = symbol_rootP; symp; symp = symbol_next (symp))
	{
	  struct elf_obj_sy *sy_obj = symbol_get_obj (symp);
	  int is_defined = !!S_IS_DEFINED (symp);

	  if (sy_obj->versioned_name)
	    {
	      char *p = strchr (sy_obj->versioned_name->name,
				ELF_VER_CHR);

	      if (sy_obj->rename)
		{
		  /* The @@@ syntax is a special case. If the symbol is
		     not defined, 2 `@'s will be removed from the
		     versioned_name. Otherwise, 1 `@' will be removed.   */
		  size_t l = strlen (&p[3]) + 1;
		  memmove (&p[1 + is_defined], &p[3], l);
		}

	      if (!is_defined)
		{
		  /* Verify that the name isn't using the @@ syntax--this
		     is reserved for definitions of the default version
		     to link against.  */
		  if (!sy_obj->rename && p[1] == ELF_VER_CHR)
		    {
		      as_bad (_("invalid attempt to declare external "
				"version name as default in symbol `%s'"),
			      sy_obj->versioned_name->name);
		      return;
		    }

		  /* Only one version symbol is allowed for undefined
		     symbol.  */
		  if (sy_obj->versioned_name->next)
		    {
		      as_bad (_("multiple versions [`%s'|`%s'] for "
				"symbol `%s'"),
			      sy_obj->versioned_name->name,
			      sy_obj->versioned_name->next->name,
			      S_GET_NAME (symp));
		      return;
		    }

		  sy_obj->rename = true;
		}
	    }

	  /* If there was .symver or .weak, but symbol was neither
	     defined nor used anywhere, remove it.  */
	  if (!is_defined
	      && (sy_obj->versioned_name || S_IS_WEAK (symp))
	      && symbol_used_p (symp) == 0
	      && symbol_used_in_reloc_p (symp) == 0)
	    symbol_remove (symp, &symbol_rootP, &symbol_lastP);
	}
    }
}

/**
 * @Description: 处理ELF格式的符号表中的一个符号
 */
void
elf_frob_symbol (symbolS *symp, int *puntp) {
    struct elf_obj_sy *sy_obj;
    expressionS *size;
    struct elf_versioned_name_list *versioned_name;

    sy_obj = symbol_get_obj (symp);

    size = sy_obj->size;
    if (size != NULL) {
        // 解析符号
        if (resolve_expression (size)
        && size->X_op == O_constant)
            S_SET_SIZE (symp, size->X_add_number);
        else {
            if (!flag_allow_nonconst_size)
                as_bad (_(".size expression for %s "
                    "does not evaluate to a constant"), S_GET_NAME (symp));
            else
                as_warn (_(".size expression for %s "
                    "does not evaluate to a constant"), S_GET_NAME (symp));
        }
        free (sy_obj->size);
        sy_obj->size = NULL;
    }
    versioned_name = sy_obj->versioned_name;
    
    /* Double check weak symbols.  */
    if (S_IS_WEAK (symp)) {
        if (S_IS_COMMON (symp))
            as_bad (_("symbol `%s' can not be both weak and common"),
		S_GET_NAME (symp));
    }
}

static struct group_list groups;
static void
build_additional_section_info (bfd *abfd ATTRIBUTE_UNUSED,
				  asection *sec, void *inf) {
    struct group_list *list = (struct group_list *) inf;
    const char *group_name = elf_group_name (sec);
    unsigned int i;
    unsigned int *elem_idx;
    unsigned int *idx_ptr;

    if (sec->map_head.linked_to_symbol_name) {
        symbolS *linked_to_sym;
        linked_to_sym = symbol_find (sec->map_head.linked_to_symbol_name);
        if (!linked_to_sym || !S_IS_DEFINED (linked_to_sym))
            as_bad (_("undefined linked-to symbol `%s' on section `%s'"),
                sec->map_head.linked_to_symbol_name,
                bfd_section_name (sec));
        else
        elf_linked_to_section (sec) = S_GET_SEGMENT (linked_to_sym);
    }

    if (group_name == NULL)
        return;

    /* If this group already has a list, add the section to the head of
        the list.  */
    elem_idx = (unsigned int *) str_hash_find (list->indexes, group_name);
    if (elem_idx != NULL) {
        elf_next_in_group (sec) = list->head[*elem_idx];
        list->head[*elem_idx] = sec;
        return;
    }

    /* New group.  Make the arrays bigger in chunks to minimize calls to
        realloc.  */
    i = list->num_group;
    if ((i & 127) == 0) {
        unsigned int newsize = i + 128;
        list->head = XRESIZEVEC (asection *, list->head, newsize);
    }
    list->head[i] = sec;
    list->num_group += 1;

    /* Add index to hash.  */
    idx_ptr = XNEW (unsigned int);
    *idx_ptr = i;
    str_hash_insert (list->indexes, group_name, idx_ptr, 0);
}

static void
free_section_idx (void *ent)
{
  string_tuple_t *tuple = ent;
  free ((char *) tuple->value);
}


/**
 * @Description: Create symbols for group signature
 */
void
elf_adjust_symtab (void) {
    unsigned int i;

    /* Go find section groups.  */
    groups.num_group = 0;
    groups.head = NULL;
    groups.indexes = htab_create_alloc (16, hash_string_tuple, eq_string_tuple,
                        free_section_idx, notes_calloc, NULL);
    bfd_map_over_sections (stdoutput, build_additional_section_info,
                &groups);
    for (i = 0; i < groups.num_group; i++) {
        const char *group_name = elf_group_name (groups.head[i]);
        const char *sec_name;
        asection *s;
        flagword flags;
        struct symbol *sy;

        flags = SEC_READONLY | SEC_HAS_CONTENTS | SEC_IN_MEMORY | SEC_GROUP;
        for (s = groups.head[i]; s != NULL; s = elf_next_in_group (s))
            if ((s->flags ^ flags) & SEC_LINK_ONCE) {
                flags |= SEC_LINK_ONCE | SEC_LINK_DUPLICATES_DISCARD;
                if (s != groups.head[i]) {
                    as_warn (_("assuming all members of group `%s' are COMDAT"),
                        group_name);
                    break;
                }
            }

        sec_name = ".group";
        s = subseg_force_new (sec_name, 0);
        if (s == NULL
            || !bfd_set_section_flags (s, flags)
            || !bfd_set_section_alignment (s, 2)) {
            as_fatal (_("can't create group: %s"),
                    bfd_errmsg (bfd_get_error ()));
        }
        elf_section_type (s) = SHT_GROUP;

        /* Pass a pointer to the first section in this group.  */
        elf_next_in_group (s) = groups.head[i];
        elf_sec_group (groups.head[i]) = s;
        /* Make sure that the signature symbol for the group has the
        name of the group.  */
        sy = symbol_find_exact (group_name);
        if (!sy || !symbol_on_chain (sy, symbol_rootP, symbol_lastP)) {
        /* Create the symbol now.  */
            sy = symbol_new (group_name, now_seg, frag_now, 0);
            symbol_get_obj (sy)->local = 1;
            symbol_table_insert (sy);
        }
        elf_group_id (s) = symbol_get_bfdsym (sy);
        /* Mark the group signature symbol as used so that it will be
        included in the symbol table.  */
        symbol_mark_used_in_reloc (sy);
    }
}

static void
adjust_stab_sections (bfd *abfd, asection *sec, void *xxx ATTRIBUTE_UNUSED) {
    char *name;
    asection *strsec;
    char *p;
    int strsz, nsyms;
    if (!startswith (sec->name, ".stab"))
        return;
}

void
elf_frob_file (void) {
    bfd_map_over_sections (stdoutput, adjust_stab_sections, NULL);
}

void
elf_fixup_removed_symbol (symbolS **sympp) {
    symbolS *symp = *sympp;
    struct elf_obj_sy *sy_obj = symbol_get_obj (symp);

    /* Replace the removed symbol with the versioned symbol.  */
    symp = symbol_find (sy_obj->versioned_name->name);
    *sympp = symp;
}

void
elf_frob_file_after_relocs (void) {
    unsigned int i;

    /* Set SHT_GROUP section size.  */
    for (i = 0; i < groups.num_group; i++) {
         asection *s, *head, *group;
        bfd_size_type size;

        head = groups.head[i];
        size = 4;
        for (s = head; s != NULL; s = elf_next_in_group (s))
            size += (s->flags & SEC_RELOC) != 0 ? 8 : 4;

        group = elf_sec_group (head);
        subseg_set (group, 0);
        bfd_set_section_size (group, size);
        group->contents = (unsigned char *) frag_more (size);
        frag_now->fr_fix = frag_now_fix_octets ();
        frag_wane (frag_now);
    }
}

struct recorded_attribute_info {
    struct recorded_attribute_info *next;
    int vendor;
    unsigned int base;
    unsigned long mask;
};

static struct recorded_attribute_info *recorded_attributes;

static void
record_attribute (int vendor, unsigned int tag)
{
    unsigned int base;
    unsigned long mask;
    struct recorded_attribute_info *rai;

    base = tag / (8 * sizeof (rai->mask));
    mask = 1UL << (tag % (8 * sizeof (rai->mask)));
    for (rai = recorded_attributes; rai; rai = rai->next)
        if (rai->vendor == vendor && rai->base == base)
        {
        rai->mask |= mask;
        return;
        }

    rai = XNEW (struct recorded_attribute_info);
    rai->next = recorded_attributes;
    rai->vendor = vendor;
    rai->base = base;
    rai->mask = mask;
    recorded_attributes = rai;
}

/**
 * @Description: 解析.comm符号
 */
symbolS *
elf_common_parse (int ignore ATTRIBUTE_UNUSED, symbolS *symbolP, addressT size) {
    addressT align = 0;
    int is_local = symbol_get_obj (symbolP)->local;

    if (*input_line_pointer == ',') {
        char *save = input_line_pointer;

        input_line_pointer++;
        SKIP_WHITESPACE ();

        if (*input_line_pointer == '"') {

        } else {
            input_line_pointer = save;
            align = parse_align (is_local);
            if (align == (addressT) -1)
                return NULL;
        }
    }
    if (is_local) {
    }
    else {
        S_SET_VALUE (symbolP, size);
        S_SET_ALIGN (symbolP, align);
        S_SET_EXTERNAL (symbolP);
        S_SET_SEGMENT (symbolP, elf_com_section_ptr);
    }

    symbol_get_bfdsym (symbolP)->flags |= BSF_OBJECT;

    return symbolP;
}

void
obj_elf_common (int is_common) {
    s_comm_internal (0, elf_common_parse);
}

/**
 * @Description: 获取读文件中的符号
 */
static symbolS *
get_sym_from_input_line_and_check (void) {
    char *name;
    char c;
    symbolS *sym;

    // 获取到最后一个符号
    c = get_symbol_name (& name);
    sym = symbol_find_or_make (name);
    // 移动读行指针
    *input_line_pointer = c;
    SKIP_WHITESPACE_AFTER_NAME ();

    /* There is no symbol name if input_line_pointer has not moved.  */
    if (name == input_line_pointer)
        as_bad (_("Missing symbol name in directive"));
    return sym;
}

static char *
obj_elf_type_name (char *cp) {
    char *p;
    p = input_line_pointer;
    if (*input_line_pointer >= '0'
      && *input_line_pointer <= '9') {
        while (*input_line_pointer >= '0'
            && *input_line_pointer <= '9')
            ++input_line_pointer;
        *cp = *input_line_pointer;
        *input_line_pointer = '\0';
    } else
        *cp = get_symbol_name (&p);

    return p;
}
/**
 * @Description: 解析.type
 */
static void
obj_elf_type (int ignore ATTRIBUTE_UNUSED) {
    char c;
    int type;
    const char *type_name;
    symbolS *sym;
    elf_symbol_type *elfsym;

    sym = get_sym_from_input_line_and_check ();
    c = *input_line_pointer;
    // 转为elf中的symbol
    elfsym = (elf_symbol_type *) symbol_get_bfdsym (sym);

    if (*input_line_pointer == ',')
        ++input_line_pointer; 
    SKIP_WHITESPACE ();

    if (   *input_line_pointer == '#'
      || *input_line_pointer == '@'
      || *input_line_pointer == '"'
      || *input_line_pointer == '%')
        ++input_line_pointer;
    // 获取类型
    type_name = obj_elf_type_name (& c);

    type = 0;
    if (strcmp (type_name, "function") == 0
        || strcmp (type_name, "2") == 0
        || strcmp (type_name, "STT_FUNC") == 0)
        type = BSF_FUNCTION;
    else if (strcmp (type_name, "object") == 0
	   || strcmp (type_name, "1") == 0
	   || strcmp (type_name, "STT_OBJECT") == 0)
        type = BSF_OBJECT;
    else
        as_bad (_("unrecognized symbol type \"%s\""), type_name);
    *input_line_pointer = c;

    if (*input_line_pointer == '"')
        ++input_line_pointer;
    flagword mask = BSF_FUNCTION | BSF_OBJECT;

    if (type != BSF_FUNCTION)
        mask |= BSF_GNU_INDIRECT_FUNCTION;
    if (type != BSF_OBJECT) {
        mask |= BSF_GNU_UNIQUE | BSF_THREAD_LOCAL;

	  if (S_IS_COMMON (sym)) {
            as_bad (_("cannot change type of common symbol '%s'"),
                S_GET_NAME (sym));
            mask = type = 0;
	    }
    }

    if (type) {
        flagword new = (elfsym->symbol.flags & ~mask) | type;

        if (new != (elfsym->symbol.flags | type))
            as_warn (_("symbol '%s' already has its type set"), S_GET_NAME (sym));
        elfsym->symbol.flags = new;
    }

    demand_empty_rest_of_line ();
}

/**
 * @Description: 解析.size
 */
static void
obj_elf_size (int ignore ATTRIBUTE_UNUSED) {
    char *name;
    char c = get_symbol_name (&name);

    char *p;
    expressionS exp;
    symbolS *sym;

    p = input_line_pointer;
    *p = c;
    SKIP_WHITESPACE_AFTER_NAME ();
    input_line_pointer++;
    expression (&exp);

    *p = 0;
    sym = symbol_find_or_make (name);
    *p = c;

    if (exp.X_op == O_constant) {
        S_SET_SIZE (sym, exp.X_add_number);
        xfree (symbol_get_obj (sym)->size);
        symbol_get_obj (sym)->size = NULL;
    }
  else {
        symbol_get_obj (sym)->size = XNEW (expressionS);
        *symbol_get_obj (sym)->size = exp;
    }

    demand_empty_rest_of_line ();
}

