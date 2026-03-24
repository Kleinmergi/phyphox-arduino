#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Button::setOutputChannel(int b)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(b, 5, "setOutputChannel") : ERROR;
    char tmp[20];
    sprintf(tmp, "CB%i", b);
    copyToMem(&OUTPUTBUFFER, tmp);
}

void PhyphoxBleExperiment::Button::setChannel(int b)
{
    setOutputChannel(b);
}

void PhyphoxBleExperiment::Button::setValue(float value)
{
    char tmp[20];
    snprintf(tmp, sizeof(tmp), "%.6g", value);
    copyToMem(&VALUE, tmp);
}

void PhyphoxBleExperiment::Button::setXMLAttribute(const char *xml)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
    copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Button::getBytes(char *buffArray)
{
    strcat(buffArray, "\t\t<button");
    if (LABEL)
    {
        strcat(buffArray, " label=\"");
        strcat(buffArray, LABEL);
        strcat(buffArray, "\"");
    }
    else
    {
        strcat(buffArray, " label=\"button\"");
    }
    if (XMLAttribute)
    {
        strcat(buffArray, XMLAttribute);
    }
    strcat(buffArray, ">\n");
    strcat(buffArray, "\t\t\t<input type=\"value\">");
    if (!VALUE)
    {
        strcat(buffArray, "1");
    }
    else
    {
        strcat(buffArray, VALUE);
    }
    strcat(buffArray, "</input>\n");
    strcat(buffArray, "\t\t\t<output>");
    if (!OUTPUTBUFFER)
    {
        strcat(buffArray, "CB1");
    }
    else
    {
        strcat(buffArray, OUTPUTBUFFER);
    }
    strcat(buffArray, "</output>\n");
    strcat(buffArray, "\t\t</button>\n");
}
