#include "phyphoxBleExperiment.h"
#include "copyToMem.h"

void PhyphoxBleExperiment::Slider::setUnit(const char *u)
{
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(u, 12, "setUnit") : ERROR;
	copyToMem(&UNIT, (std::string(u)).c_str());
}

void PhyphoxBleExperiment::Slider::setMin(float min)
{
	char tmp[20];
	sprintf(tmp, "%g", min);
	copyToMem(&MIN, tmp);
}

void PhyphoxBleExperiment::Slider::setMax(float max)
{
	char tmp[20];
	sprintf(tmp, "%g", max);
	copyToMem(&MAX, tmp);
}

void PhyphoxBleExperiment::Slider::setStep(float step)
{
	char tmp[20];
	sprintf(tmp, "%g", step);
	copyToMem(&STEP, tmp);
}

void PhyphoxBleExperiment::Slider::setValue(float value)
{
	char tmp[20];
	sprintf(tmp, "%g", value);
	copyToMem(&VALUE, tmp);
}

void PhyphoxBleExperiment::Slider::setXMLAttribute(const char *xml){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Slider::setChannel(int b){
	ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(b, 5, "setChannel") : ERROR;
	char tmp[20];
	sprintf(tmp, "CB%i", b);
	copyToMem(&BUFFER, tmp);
}

void PhyphoxBleExperiment::Slider::getBytes(char *buffArray)
{
	strcat(buffArray,"\t\t<slider");
	if(LABEL){
		strcat(buffArray," label=\"");
		strcat(buffArray,LABEL);
		strcat(buffArray,"\"");
	} else {
		strcat(buffArray," label=\"slider\"");
	}

	if(UNIT){
		strcat(buffArray," unit=\"");
		strcat(buffArray,UNIT);
		strcat(buffArray,"\"");
	}
	if(MIN){
		strcat(buffArray," minValue=\"");
		strcat(buffArray,MIN);
		strcat(buffArray,"\"");
	}
	if(MAX){
		strcat(buffArray," maxValue=\"");
		strcat(buffArray,MAX);
		strcat(buffArray,"\"");
	}
	if(STEP){
		strcat(buffArray," stepSize=\"");
		strcat(buffArray,STEP);
		strcat(buffArray,"\"");
	}
	if(VALUE){
		strcat(buffArray," default=\"");
		strcat(buffArray,VALUE);
		strcat(buffArray,"\"");
	}
	if (XMLAttribute) {strcat(buffArray,XMLAttribute);}
	strcat(buffArray,">\n");
	strcat(buffArray,"\t\t<output>");
	if (!BUFFER)  {strcat(buffArray,"CH5");} else {strcat(buffArray,BUFFER);}
	strcat(buffArray,"</output>\n");
	strcat(buffArray,"\t\t</slider>\n");
}
