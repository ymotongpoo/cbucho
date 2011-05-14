#include <Python.h>
#include <curl/curl.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "cbuchomodule.h"

#define DEBUG(...) \
    do {                   \
        printf("DEBUG: ");   \
        printf(__VA_ARGS__); \
        printf("\n");        \
    } while(0)
#define DUMP(...)                               \
    do {                                                                \
        printf("-------------------------------------------------------\n"); \
        printf(__VA_ARGS__);                                            \
        printf("\n");                                                   \
        printf("-------------------------------------------------------\n"); \
    

/* utility functions */
Memory*
malloc_memory(void) 
{
    Memory* mem;
    mem = (Memory*)PyMem_Malloc(sizeof(Memory));
    memset(mem, 0, sizeof(Memory));
    //DEBUG("malloc_memory %p", mem);
    return mem;
}


void
free_memory(Memory* mem) 
{
    //DEBUG("free_memory %p", mem);
    PyMem_Free(mem);
}


Memory*
new_memory(size_t size, size_t limit)
{
    Memory* mem;

    mem = malloc_memory();
    if (!mem) {
        PyErr_NoMemory();
        return NULL;
    }

    mem->memory = (char*)PyMem_Malloc(sizeof(char) * size);
    if (!mem->memory) {
        PyErr_NoMemory();
        return NULL;
    }
    //DEBUG("new_memory %p (memory %p)", mem, mem->memory);

    mem->size = size;
    mem->limit = limit;

    return mem;
}


int
copy_to_memory(Memory* mem, const char* c, size_t l) 
{
    char* new_memory;
    size_t new_len;

    if (mem == NULL) {
        PyErr_NoMemory();
        return 1;
    }

    new_len = (int)(mem->len + l);
    if (new_len >= mem->size) {
        mem->size *= 2;
        if (new_len >= mem->size) {
            mem->size = new_len + 1;
        }
        if (mem->size > mem->limit) {
            mem->size = mem->limit + l;
        }
        new_memory = (char*)PyMem_Realloc(mem->memory, mem->size);
        if (new_memory == NULL) {
            PyErr_SetString(PyExc_MemoryError, "out of memory");
            PyMem_Free(mem->memory);
            mem->memory = NULL;
            mem->size = mem->len = 0;
            return 1;
        }

        mem->memory = new_memory;
    }

    memcpy(mem->memory + mem->len, c, l);
    
    mem->len = new_len;
    return 0;
}


/**
   callback function for curl_easy_setopt()
   
   return:
     success: size of size & nmemb
     failure: -1
 */
size_t
write_memory_callback(void* ptr, size_t size, size_t nmemb, void* data)
{
    if (size * nmemb == 0) {
        return 0;
    }
    size_t realsize = size * nmemb;
    Memory* mem = (Memory*)data;
    copy_to_memory(mem, ptr, realsize);
    return realsize;
}
            
/**
   return:
     success: 0
     failure: 1
 */
int
expr_xpath_text_from_string(Memory* mem, char* xpath, Memory* ret)
{
    xmlDocPtr doc = NULL;
    xmlXPathContextPtr ctx = NULL;
    xmlXPathObjectPtr xpobjp = NULL;
    xmlNodeSetPtr nsp = NULL;
    int node_size;
    xmlNodePtr np = NULL;
    int i;
    size_t ret_size = strlen(ret);
    size_t content_size = 0;

    if (!mem) {
        PyErr_SetString(PyExc_MemoryError, "out of memory");
        return 1;
    }
        
    //DEBUG("expr_xpath_text_from_string: memory\n %s", mem->memory);
    doc = xmlParseMemory(mem->memory, mem->size);
    if (!doc) return 1;

    ctx = xmlXPathNewContext(doc);
    if (!ctx) return 1;

    xpobjp = xmlXPathEvalExpression((xmlChar*)xpath, ctx);
    if (!xpobjp) return 1;

    nsp = xpobjp->nodesetval;
    node_size = (nsp) ? nsp->nodeNr : 0;
    for (i = 0; i < node_size; ++i) {
        if (nsp->nodeTab[i]->type == XML_ELEMENT_NODE) {
            np = nsp->nodeTab[i];

            content_size = strlen(np->children->content);
            copy_to_memory(ret, (char*)(np->children->content), content_size);
            copy_to_memory(ret, "\n", 1);
        }
    }

    xmlXPathFreeObject(xpobjp);
    xmlXPathFreeContext(ctx);
    xmlFreeDoc(doc);
    xmlCleanupParser();

    return 0;
}

/**
   return:
     success: 0
     failure: 1
 */
int 
get_xml_content(Memory* mem, char* url)
{
    CURL* curl;
    CURLcode result = 0;
    
    curl = curl_easy_init();

    if (!curl) {
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)mem);

    result = curl_easy_perform(curl);
    //DEBUG("get_xml_content: memory\n %s", mem->memory);
    curl_easy_cleanup(curl);
    
    return result;
}


/* module Functions */
static PyObject *
cbucho_system(PyObject *self, PyObject *args) 
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command)) {
        return NULL;
    }

    sts = system(command);
    return Py_BuildValue("i", sts);
}


static PyObject *
cbucho_show(PyObject *self)
{
    return PyString_FromString(_show_text);
}


static PyObject *
cbucho_latest_status(PyObject *self)
{
    Memory* mem = new_memory(DEFAULT_SIZE, LIMIT_MAX);
    Memory* ret = new_memory(DEFAULT_SIZE, LIMIT_MAX);
    int result;

    result = get_xml_content(mem, _bucho_latest_twitter_url);
    copy_to_memory(mem, "\0", 1);
    if (result) {
        PyErr_SetString("[error] in get_xml_content() : %d\n", result);
    }

    result = expr_xpath_text_from_string(mem, "//status/text", ret);
    if (result) {
        PyErr_SetString("[error] in expr_xpath_text_from_string() : %d\n", result);
    }

    free(mem->memory);
    mem->memory = NULL;
    free(mem);

    return PyString_FromString(ret->memory);
}


static PyObject *
cbucho_all_status(PyObject *self)
{
    Memory* mem = new_memory(DEFAULT_SIZE, LIMIT_MAX);
    Memory* ret = new_memory(DEFAULT_SIZE, LIMIT_MAX);
    int result;

    result = get_xml_content(mem, _bucho_all_twitter_url);
    copy_to_memory(mem, "\0", 1);
    if (result) {
        PyErr_SetString("[error] in get_xml_content() : %d\n", result);
    }

    result = expr_xpath_text_from_string(mem, "//status/text", ret);
    if (result) {
        PyErr_SetString("[error] in get_xml_content() : %d\n", result);
    }

    free(mem->memory);
    mem->memory = NULL;
    free(mem);

    return PyString_FromString(ret->memory);
}


/* methods */
static PyMethodDef cbucho_methods[] = {
    {"system", cbucho_system, METH_VARARGS,
     "execute a shell command"},
    {"show", cbucho_show, METH_VARARGS,
     "show"},
    {"latest_status", cbucho_latest_status, METH_VARARGS,
     "print bucho's latest status"},
    {"all_status", cbucho_all_status, METH_VARARGS,
     "print bucho's last 20 statuses"},
    {NULL, NULL},
};



void
initcbucho(void)
{
    Py_InitModule("cbucho", cbucho_methods);
}

