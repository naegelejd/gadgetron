#pragma once

#include "python_toolbox.h"
#include "log.h"

#include <set>

/* #include "setobject.h" */
#include <boost/python.hpp>
namespace bp = boost::python;

namespace Gadgetron {

template <typename T>
struct set_to_python_set {

    static PyObject* convert(const std::set<T>& s)
    {
        PyObject* pyset = PySet_New(NULL);
        if (nullptr == pyset) {
            GERROR("failed to create new Python set object\n");
            return NULL;
        }

        typename std::set<T>::const_iterator it;
        for (it = s.begin(); it != s.end(); ++it) {
            // TODO: verify reference counting of new set element
            int ret = PySet_Add(pyset, bp::object(*it).ptr());
            if (0 != ret) {
                // TODO: check Python exception on failure
                GERROR("failed to add item to Python set object\n");
                // TODO: cleanup pyset or let garbage collector do it?
                return NULL;
            }
        }

        return bp::incref(pyset);
    }
};

template <typename T>
struct set_from_python_set {
    set_from_python_set() {
        bp::converter::registry::push_back(
                &convertible,
                &construct,
                bp::type_id<std::set<T> >());
    }

    static void* convertible(PyObject* obj) {
        // TODO: this *could* check that every element in the set is of the
        // correct type, a requirement of C++ std::set
        if (!PySet_Check(obj)) {
            return NULL;
        }
        return obj;
    }

    static void construct(PyObject* obj, bp::converter::rvalue_from_python_stage1_data* data) {
        void* storage = ((bp::converter::rvalue_from_python_storage<std::set<T> >*)data)->storage.bytes;
        data->convertible = storage;

        std::set<T>* s = new (storage) std::set<T>;

        while (PySet_Size(obj) > 0) {
            PyObject* item = PySet_Pop(obj);
            if (NULL == item) {
                GERROR("failed to pop item from Python set object\n");
                return;
            }

            try {
                s->insert(bp::extract<T>(item));
            } catch (const bp::error_already_set&) {
                GERROR("failed to extract and insert item into Python set object\n");
                return;
            }
        }
    }
};

/// Create and register set converter as necessary
template <typename T> void create_set_converter() {
    bp::type_info info = bp::type_id<std::set<T> >();
    const bp::converter::registration* reg = bp::converter::registry::query(info);
    // only register if not already registered!
    if (nullptr == reg || nullptr == (*reg).m_to_python) {
        bp::to_python_converter<std::set<T>, set_to_python_set<T> >();
        set_from_python_set<T>();
    }
}

/// Partial specialization of `python_converter` for std::set
template <typename T>
struct python_converter<std::set<T> > {
    static void create()
    {
        create_set_converter<T>();
    }
};

}
