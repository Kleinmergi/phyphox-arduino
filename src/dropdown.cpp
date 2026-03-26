#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Dropdown::setOutputChannel(int b)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(b, 5, "setOutputChannel") : ERROR;
    char tmp[20];
    sprintf(tmp, "CB%i", b);
    copyToMem(&OUTPUTBUFFER, tmp);
}

void PhyphoxBleExperiment::Dropdown::setChannel(int b)
{
    setOutputChannel(b);
}

void PhyphoxBleExperiment::Dropdown::addOption(const char *label, float value)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(label, 41, "addOption") : ERROR;

    for (int i = 0; i < phyphoxBleNChannel; i++)
    {
        if (OPTIONLABELS[i] == NULL)
        {
            copyToMem(&OPTIONLABELS[i], label);
            char tmp[20];
            snprintf(tmp, sizeof(tmp), "%.6g", value);
            copyToMem(&OPTIONVALUES[i], tmp);
            break;
        }
    }
}

void PhyphoxBleExperiment::Dropdown::setXMLAttribute(const char *xml)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
    copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Dropdown::getBytes(char *buffArray)
{
    strcat(buffArray, "\t\t<dropdown");
    if (LABEL)
    {
        strcat(buffArray, " label=\"");
        strcat(buffArray, LABEL);
        strcat(buffArray, "\"");
    }
    else
    {
        strcat(buffArray, " label=\"dropdown\"");
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

    for (int i = 0; i < phyphoxBleNChannel; i++)
    {
        if (OPTIONLABELS[i] != NULL)
        {
            strcat(buffArray, "\t\t\t<map value=\"");
            strcat(buffArray, OPTIONVALUES[i]);
            strcat(buffArray, "\">");
            strcat(buffArray, OPTIONLABELS[i]);
            strcat(buffArray, "</map>\n");
        }
    }

    strcat(buffArray, "\t\t</dropdown>\n");
}
