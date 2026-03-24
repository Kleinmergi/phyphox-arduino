#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Toggle::setDefault(bool d)
{
    if (d)
        copyToMem(&DEFAULTVALUE, "true");
    else
        copyToMem(&DEFAULTVALUE, "false");
}

void PhyphoxBleExperiment::Toggle::setOutputChannel(int b)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(b, 5, "setOutputChannel") : ERROR;
    char tmp[20];
    sprintf(tmp, "CB%i", b);
    copyToMem(&OUTPUTBUFFER, tmp);
}

void PhyphoxBleExperiment::Toggle::setChannel(int b)
{
    setOutputChannel(b);
}

void PhyphoxBleExperiment::Toggle::setXMLAttribute(const char *xml)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
    copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Toggle::getBytes(char *buffArray)
{
    strcat(buffArray, "\t\t<toggle");
    if (LABEL)
    {
        strcat(buffArray, " label=\"");
        strcat(buffArray, LABEL);
        strcat(buffArray, "\"");
    }
    else
    {
        strcat(buffArray, " label=\"toggle\"");
    }
    if (DEFAULTVALUE)
    {
        strcat(buffArray, " default=\"");
        strcat(buffArray, DEFAULTVALUE);
        strcat(buffArray, "\"");
    }
    if (XMLAttribute)
    {
        strcat(buffArray, XMLAttribute);
    }
    strcat(buffArray, ">\n");
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
    strcat(buffArray, "\t\t</toggle>\n");
}
