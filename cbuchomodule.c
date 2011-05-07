#include <Python.h>
#include <curl/curl.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include "cbuchomodule.h"

/* utility functions */

typedef struct
{
    char* memory;
    size_t size;
} Memory;

/**
   callback function for curl_easy_setopt()
 */
size_t
write_memory_callback(void* ptr, size_t size, size_t nmemb, void* data)
{
    if (size * nmemb == 0)
        return 0;

    size_t realsize = size * nmemb;
    Memory* mem = (Memory *)data;
    
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
     success: string content specified with XPath
     failure: 0
 */
char*
find_xpath_text_from_char(Memory* mem, char* xpath)
{
    xmlDocPtr doc = NULL;
    xmlXPathContextPtr ctx = NULL;
    xmlXPathObjectPtr xpobjp = NULL;
    xmlNodeSetPtr nsp = NULL;
    int node_size;
    xmlNodePtr np = NULL;
    int i;

    char* result = NULL;
    int result_size;
    char* tmp = NULL;

    if (!mem->memory) {
        printf("memory is NULL\n");
        return 0;
    }
        
    printf("size : %d\n%s", (int)mem->size, mem->memory);
    //doc = xmlParseMemory(mem->memory, mem->size);
    printf("%s", "foo0\n");
    doc = xmlParseMemory("<?xml version=\"1.0\" ?><hoge><piyo>foo</piyo><piyo>bar</piyo></hoge>", 
                         sizeof("<?xml version=\"1.0\" ?><hoge><piyo>foo</piyo><piyo>bar</piyo></hoge>"));
    if (!doc) return 0;

    printf("%s", "foo1\n");
    ctx = xmlXPathNewContext(doc);
    if (!ctx) return 0;

    printf("%s", "foo2\n");
    xpobjp = xmlXPathEvalExpression((xmlChar*)xpath, ctx);
    if (!xpobjp) return 0;

    printf("%s", "foo3\n");
    nsp = xpobjp->nodesetval;

    node_size = (nsp) ? nsp->nodeNr : 0;
    printf("%d\n", node_size);
    /*
    for (i = 0; i < node_size; i++) {
        if (nsp->nodeTab[i]->type == XML_ELEMENT_NODE) {
            np = nsp->nodeTab[i];
            result_size = sizeof(result) + sizeof((char*)np->content) + 1;
            tmp = (char*)realloc(result, result_size);
            if ( !strcat(result, (char*)np->content) ) {
                return 0;
            }
        }
    }
    */
    return result;
}

/**
   return:
     success: HTTP response data
     failure: 0
 */
Memory*
get_xml_content(char* url)
{
    CURL* curl;
    CURLcode res;
    Memory tmp = {0};
    Memory* mem = &tmp;
    
    curl = curl_easy_init();

    if (!curl)
        return 0;

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)mem);

    res = curl_easy_perform(curl);
    if (res)
        return 0;
    curl_easy_cleanup(curl);

    return mem;
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
    Memory* mem;
    char* status;

    mem = get_xml_content(_bucho_twitter_url);

    if (mem) {
        printf("%s", "piyo1\n");
        status = find_xpath_text_from_char(mem, "//piyo");
        printf("%s", "piyo2\n");
        printf("%s", status);
    }

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

