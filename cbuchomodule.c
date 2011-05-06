#include <Python.h>
#include <curl/curl.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "cbuchomodule.h"

#define MAX_BUF 65536


/* utility functions */

typedef struct
{
    char *memory;
    size_t size;
} Memory;

/**
   callback function for curl_easy_setopt()
 */
size_t
write_memory_callback(void *ptr, size_t size, size_t nmemb, void *data)
{
    if (size * nmemb == 0)
        return 0;

    size_t realsize = size * nmemb;
    Memory *mem = (Memory *) data;
    
    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    if (!mem->memory)
        return -1;

    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}
            

/**
   return:
     success: HTTP response data
     failure: -1
 */
int
get_xml_content(char* url)
{
    CURL* curl;
    CURLcode res;

    curl = curl_easy_init();

    if (!curl)
        return -1;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    Memory mem = {0};
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mem);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    printf("%s", mem.memory);

    return res;
}


int
find_xpath_text_from_char(Memory mem, char* xpath)
{
    xmlDocPtr doc = xmlParseMemory(mem.memory, mem.size);
    if (!doc) return -1;

    xmlXPathContextPtr ctx = xmlXPathNewContext(doc);
    if (!ctx) return -1;

    xmlXPathObjectPtr xpobj = xmlXPathEvalExpression((xmlChar*) xpath, ctx);
    if (!xpobj) return -1;
    
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
    CURLcode res;
    res = get_xml_content(_bucho_twitter_url);

    Py_RETURN_NONE;
}

/* methods */
static PyMethodDef cbucho_methods[] = {
    {"system", cbucho_system, METH_VARARGS,
     "execute a shell command"},
    {"show", cbucho_show, METH_NOARGS,
     "show"},
    {"latest_status", cbucho_latest_status, METH_NOARGS,
     "print latest bucho's status"},
    {NULL, NULL},
};



void
initcbucho(void)
{
    Py_InitModule("cbucho", cbucho_methods);
}

