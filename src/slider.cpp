#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Slider::setMin(float min)
{
    char tmp[20];
    snprintf(tmp, sizeof(tmp), "%.6g", min);
    copyToMem(&MIN, tmp);
}

void PhyphoxBleExperiment::Slider::setMax(float max)
{
    char tmp[20];
    snprintf(tmp, sizeof(tmp), "%.6g", max);
    copyToMem(&MAX, tmp);
}

void PhyphoxBleExperiment::Slider::setStep(float step)
{
    char tmp[20];
    snprintf(tmp, sizeof(tmp), "%.6g", step);
    copyToMem(&STEP, tmp);
}

void PhyphoxBleExperiment::Slider::setValue(float value)
{
    char tmp[20];
    snprintf(tmp, sizeof(tmp), "%.6g", value);
    copyToMem(&VALUE, tmp);
}

void PhyphoxBleExperiment::Slider::setOutputChannel(int b)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(b, 5, "setOutputChannel") : ERROR;
    char tmp[20];
    sprintf(tmp, "CB%i", b);
    copyToMem(&OUTPUTBUFFER, tmp);
}

void PhyphoxBleExperiment::Slider::setChannel(int b)
{
    setOutputChannel(b);
}

void PhyphoxBleExperiment::Slider::setXMLAttribute(const char *xml)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
    copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Slider::getBytes(char *buffArray)
{
    strcat(buffArray, "\t\t<slider");
    if (LABEL)
    {
        strcat(buffArray, " label=\"");
        strcat(buffArray, LABEL);
        strcat(buffArray, "\"");
    }
    else
    {
        strcat(buffArray, " label=\"slider\"");
    }
    if (MIN)
    {
        strcat(buffArray, " min=\"");
        strcat(buffArray, MIN);
        strcat(buffArray, "\"");
    }
    if (MAX)
    {
        strcat(buffArray, " max=\"");
        strcat(buffArray, MAX);
        strcat(buffArray, "\"");
    }
    if (STEP)
    {
        strcat(buffArray, " step=\"");
        strcat(buffArray, STEP);
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
    if (VALUE)
    {
        strcat(buffArray, "\t\t\t<value>");
        strcat(buffArray, VALUE);
        strcat(buffArray, "</value>\n");
    }
    strcat(buffArray, "\t\t</slider>\n");
}
