#include <Python.h>
#include <curl/curl.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "cbuchomodule.h"

/* utility functions */

#define DEFAULT_BUF 1024

typedef struct
{
    char* memory;
    size_t size;
} Memory;

/**
   callback function for curl_easy_setopt()
   
   return:
     success: size of size & nmemb
     failure: -1
 */
size_t
write_memory_callback(void* ptr, size_t size, size_t nmemb, void* data)
{
    if (size * nmemb == 0)
        return 0;

    size_t realsize = size * nmemb;
    Memory* mem = (Memory*)data;
    
    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory) {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }

    return realsize;
}
            
/**
   return:
     success: 0
     failure: 1
 */
int
print_xpath_text_from_char(Memory* mem, char* xpath)
{
    xmlDocPtr doc = NULL;
    xmlXPathContextPtr ctx = NULL;
    xmlXPathObjectPtr xpobjp = NULL;
    xmlNodeSetPtr nsp = NULL;
    int node_size;
    xmlNodePtr np = NULL;
    int i;

    if ( !(mem->memory) ) {
        printf("memory is NULL\n");
        return 0;
    }
        
    doc = xmlParseMemory(mem->memory, mem->size);
    if (!doc) return 1;

    ctx = xmlXPathNewContext(doc);
    if (!ctx) return 1;

    xpobjp = xmlXPathEvalExpression((xmlChar*)xpath, ctx);
    if (!xpobjp) return 1;

    nsp = xpobjp->nodesetval;
    node_size = (nsp) ? nsp->nodeNr : 0;
    for (i = 0; i < node_size; i++) {
        if (nsp->nodeTab[i]->type == XML_ELEMENT_NODE) {
            np = nsp->nodeTab[i];
            printf("%s\n", np->children->content);
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
    CURLcode res;
    
    curl = curl_easy_init();

    if (!curl)
        return 1;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)mem);

    res = curl_easy_perform(curl);
    if (res)
        return 1;
    curl_easy_cleanup(curl);
    
    return 0;
}


/* module Functions */
static PyObject *
cbucho_system(PyObject *self, PyObject *args) 
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;

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
    Memory* mem = malloc(sizeof(Memory*));
    int res;

    mem->size = 0;
    mem->memory = NULL;

    res = get_xml_content(mem, _bucho_latest_twitter_url);
    if (res)
        printf("[error] in get_xml_content() : %d\n", res);

    res = print_xpath_text_from_char(mem, "//text");

    free(mem->memory);
    free(mem);

    Py_RETURN_NONE;
}


static PyObject *
cbucho_all_status(PyObject *self)
{
    Memory* mem = malloc(sizeof(Memory*));
    int res;

    mem->size = 0;
    mem->memory = NULL;

    res = get_xml_content(mem, _bucho_all_twitter_url);
    if (res)
        printf("[error] in get_xml_content() : %d\n", res);

    res = print_xpath_text_from_char(mem, "//text");

    free(mem->memory);
    free(mem);

    Py_RETURN_NONE;
}


/* methods */
static PyMethodDef cbucho_methods[] = {
    {"system", cbucho_system, METH_VARARGS,
     "execute a shell command"},
    {"show", cbucho_show, METH_NOARGS,
     "show"},
    {"latest_status", cbucho_latest_status, METH_NOARGS,
     "print bucho's latest status"},
	{"all_status", cbucho_all_status, METH_NOARGS,
     "print bucho's last 20 statuses"},
    {NULL, NULL},
};



void
initcbucho(void)
{
    Py_InitModule("cbucho", cbucho_methods);
}

