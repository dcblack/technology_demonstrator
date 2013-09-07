#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H
// *******************************************************************
// User-defined memory manager, which maintains a pool of transactions
// *******************************************************************

#include <tlm.h>

class memory_manager: public tlm::tlm_mm_interface {
  typedef tlm::tlm_generic_payload gp_t;
public:
  gp_t* allocate(void);
  void  free(gp_t* trans);

  static memory_manager* get_memory_manager(void) {
    if (0 == m_memory_manager_ptr) {
      m_memory_manager_ptr = new memory_manager();
    }//endif
    return m_memory_manager_ptr;
  }

private:
  static memory_manager* m_memory_manager_ptr;
  memory_manager(void) //< Constructor
  : free_list(0)
  , empties(0)
  {}

  virtual ~memory_manager(void) { //< Destructor
    gp_t* ptr;

    while (free_list) {
      ptr = free_list->trans;

      // Delete generic payload and all extensions
      assert(ptr);
      delete ptr;

      free_list = free_list->next;
    }

    while (empties) {
      access* x = empties;
      empties = empties->next;

      // Delete free list access struct
      delete x;
    }
  }

private:
  struct access {
    gp_t* trans;
    access* next;
    access* prev;
  };

  access* free_list;
  access* empties;
};

inline memory_manager::gp_t* memory_manager::allocate(void) {
  gp_t* ptr;
  if (free_list) {
    ptr = free_list->trans;
    empties = free_list;
    free_list = free_list->next;
  } else {
    ptr = new gp_t(this);
  }
  return ptr;
}

inline void memory_manager::free(gp_t* trans) {
  trans->reset(); // Delete auto extensions
  if (!empties) {
    empties = new access;
    empties->next = free_list;
    empties->prev = 0;
    if (free_list) free_list->prev = empties;
  }
  free_list = empties;
  free_list->trans = trans;
  empties = free_list->prev;
}
#endif
