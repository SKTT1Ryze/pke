#include "console.h"
#include <stdint.h>
#include "defs.h"
#include "list.h"
#include "pmm.h"
#include "encoding.h"
#include "mmap.h"
#include "bits.h"
#include "pk.h"

uintptr_t __page_alloc();

struct Page *pages;
extern uintptr_t first_free_paddr;
extern uintptr_t mem_size;
extern uintptr_t first_free_page;
int pmm_init(){
	pages= (struct Page *) __page_alloc();

   	nbase = ROUNDUP(first_free_paddr,RISCV_PGSIZE)>>RISCV_PGSHIFT;	
    pmm_manager = &default_pmm_manager;
    pmm_manager->init();
    pmm_manager->init_memmap(pa2page(first_free_paddr), mem_size/RISCV_PGSIZE);
    pmm_manager->pmm_check();
    return 0;
}

int pk_default_pmm_alloc(){
	return 0;
}


free_area_t free_area;

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)


static void
default_init(void) {
    list_init(&free_list);
    nr_free = 0;
}


static void
default_init_memmap(struct Page *base, size_t n) {
    struct Page *p = base;
    for (; p != base + n; p ++) {
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    nr_free += n;
    if (list_empty(&free_list)) {
        list_add(&free_list, &(base->page_link));
    } else {
        list_entry_t* le = &free_list;
        while ((le = list_next(le)) != &free_list) {
            struct Page* page = le2page(le, page_link);
            if (base < page) {
                list_add_before(le, &(base->page_link));
                break;
            } else if (list_next(le) == &free_list) {
                list_add(le, &(base->page_link));
            }
        }
    }
}


static struct Page *
default_alloc_pages(size_t n) {
    if (list_empty(&free_list) || nr_free < n) return NULL;
    list_entry_t* new_le = &free_list;
    while ((new_le = list_next(new_le)) != &free_list) {
        if (le2page(new_le, page_link)->property > n) {
            struct Page* new_page = le2page(new_le, page_link);
            (new_page + n) -> property = new_page -> property - n;
            new_page -> flags = 1;
            (new_page + n) -> flags = 0;
            set_page_ref((new_page + n), 0);
            list_add(list_prev(new_le), &((new_page + n) -> page_link));
            list_del(new_le);
            printk("return page: 0x%lx\n", new_page);
            nr_free -= n;
            return new_page;
        }
        else if (le2page(new_le, page_link)->property == n) {
            list_del(new_le);
            printk("return page: 0x%lx\n", le2page(new_le, page_link));
            nr_free -= n;
            return le2page(new_le, page_link);
        }
    }
    return NULL;
}


static void
default_free_pages(struct Page *base, size_t n) {
    struct Page *p = base;
    for (; p != base + n; p ++) {
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    if (list_empty(&free_list)) {
        list_add(&free_list, &(base->page_link));
    }
    else {
        list_entry_t* le = &free_list;
        while ((le = list_next(le)) != &free_list) {
            if(base < le2page(le, page_link)) {
                list_add_before(le, &(base -> page_link));
                break;
            }
            else if (list_next(le) == &free_list) {
                list_add(le, &(base->page_link));
            }
        }
    }
    nr_free += n;
}

static size_t
default_nr_free_pages(void) {
    return nr_free;
}


static void
basic_check(void) {
    struct Page *p0, *p1, *p2;
    p0 = p1 = p2 = NULL;
    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(p0 != p1 && p0 != p2 && p1 != p2);
    assert(p0->ref == 0 && p1->ref == 0 && p2->ref == 0);


    list_entry_t free_list_store = free_list;
    list_init(&free_list);
    assert(list_empty(&free_list));

    unsigned int nr_free_store = nr_free;
    nr_free = 0;
    free_page(p0);
    free_page(p1);
    free_page(p2);
    assert(nr_free == 3);

    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(alloc_page() == NULL);

    free_page(p0);
    assert(!list_empty(&free_list));

    struct Page *p;
    assert((p = alloc_page()) == p0);
    assert(alloc_page() == NULL);

    assert(nr_free == 0);
    free_list = free_list_store;
    nr_free = nr_free_store;

    free_page(p);
    free_page(p1);
    free_page(p2);
}




const struct pmm_manager default_pmm_manager = {
    .name = "default_pmm_manager",
    .init = default_init,
    .init_memmap = default_init_memmap,
    .alloc_pages = default_alloc_pages,
    .free_pages = default_free_pages,
    .nr_free_pages = default_nr_free_pages,
    .pmm_check = basic_check,
};