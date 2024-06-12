#include <nvui/window.h>
#include <nvui/painter.h>
#include <nvui/widgets.h>

#include <stdio.h>

#define NUM_COLORS 6
const Color colors[NUM_COLORS] =
{
	COLOR_CONTROL,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_WHITE,
	COLOR_BLACK
};
const Color textColors[NUM_COLORS] =
{
	COLOR_BLACK,
	COLOR_BLACK,
	COLOR_BLACK,
	COLOR_WHITE,
	COLOR_BLACK,
	COLOR_WHITE
};

Checkbox *tristateCheck;

int ButtonMessage(Element *element, Message message, int di, void *dp)
{
	static int currentColor = 0;
	if(message == MSG_BUTTON_GET_COLOR)
	{
		*(Color*)dp = colors[currentColor];
	}
	else if(message == MSG_BUTTON_GET_TEXT_COLOR)
	{
		*(Color*)dp = textColors[currentColor];
	}
	else if(message == MSG_CLICKED)
	{
		currentColor = (currentColor + 1) % NUM_COLORS;
		CheckboxSetState(tristateCheck, Indeterminate);
	}
	return 0;
}

#if 1

int CheckboxMessage(Element *element, Message message, int di, void *dp)
{
	if(message == MSG_CLICKED)
	{
		printf("Checkbox Clicked!\n");
	}
	else if(message == MSG_CHECKBOX_STATE_CHANGE)
	{
		printf("Checkbox checked: %d\n", di);
	}

	return 0;
}

int main() 
{
	Initialize();

	Window *window = WindowCreate("Hello Window", 800, 600);
	FlowPanel *contentPanel = FlowPanelCreate(WindowGetRootElement(window), 0);
	FlowPanelSetGap(contentPanel, 10);
	//FlowPanelSetBorder(contentPanel, (Rectangle){ 10, 10, 10, 10 });

	FlowPanel *column = FlowPanelCreate((Element*)contentPanel, PANEL_BORDER | ELEMENT_H_FILL | FLOWPANEL_HORIZONTAL);
	FlowPanelSetGap(column, 10);
	FlowPanelSetBorder(column, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate((Element*)column, 0, "Hello World", -1);
	Button *button = ButtonCreate((Element*)column, 0, "Test2", -1);
	ElementSetUserHandler((Element*)button, ButtonMessage);

	Checkbox *checkbox = CheckboxCreate((Element*)column, 0, "Check", -1);
	ElementSetUserHandler((Element*)checkbox, CheckboxMessage);

	tristateCheck = CheckboxCreate((Element*)column, CHECKBOX_TRISTATE, "Tristate", -1);

	return MessageLoop();
}

#else

int main()
{
	Initialize();

	Window *window1 = WindowCreate("Window 1", 1000, 800);

	FlowPanel *column1 = FlowPanelCreate(WindowGetRootElement(window1), 0);
	FlowPanelSetGap(column1, 10);
	FlowPanelSetBorder(column1, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate((Element*)column1, 0, "Label 1", -1);
	LabelCreate((Element*)column1, 0, "Longer label 2", -1);

	FlowPanel *column2 = FlowPanelCreate((Element*)column1, PANEL_BORDER | PANEL_BORDER_3D);
	FlowPanelSetGap(column2, 10);
	FlowPanelSetBorder(column2, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate((Element*)column2, ELEMENT_H_FILL, "Label 3", -1);
	LabelCreate((Element*)column2, ELEMENT_H_FILL, "Much Longer label 4", -1);

	FlowPanel *column3 = FlowPanelCreate((Element*)column1, PANEL_BORDER | PANEL_BORDER_3D | ELEMENT_H_FILL);
	FlowPanelSetGap(column2, 10);
	FlowPanelSetBorder(column2, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate((Element*)column3, 0, "Label 4", -1);
	LabelCreate((Element*)column3, 0, "Longer label 5", -1);

	FlowPanel *column4 = FlowPanelCreate((Element*)column1, PANEL_BORDER | ELEMENT_H_FILL);
	FlowPanelSetGap(column4, 10);
	FlowPanelSetBorder(column4, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate((Element*)column4, ELEMENT_H_FILL, "Label 6", -1);
	LabelCreate((Element*)column4, ELEMENT_H_FILL, "Longer label 7", -1);

	ButtonCreate((Element*)column1, ELEMENT_V_FILL, "Vertical fill button 1", -1);
	ButtonCreate((Element*)column1, ELEMENT_V_FILL, "Vertical fill button 2", -1);
	ButtonCreate((Element*)column1, ELEMENT_V_FILL | ELEMENT_H_FILL, "Vertical and horizontal fill button 3", -1);
	ButtonCreate((Element*)column1, ELEMENT_V_FILL, "Vertical fill button 4", -1);

	FlowPanel *row1 = FlowPanelCreate((Element*)column1, PANEL_BORDER | PANEL_HORIZONTAL);
	FlowPanelSetGap(row1, 10);
	FlowPanelSetBorder(row1, (Rectangle){ 10, 10, 10, 10 });

	ButtonCreate((Element*)row1, 0, "Button 1 in row", -1);
	ButtonCreate((Element*)row1, ELEMENT_H_FILL, "Button 2 in row", -1);
	ButtonCreate((Element*)row1, 0, "Button 3 in row", -1);

	FlowPanel *row2 = FlowPanelCreate((Element*)column1, PANEL_BORDER | PANEL_HORIZONTAL | ELEMENT_H_FILL);
	FlowPanelSetGap(row2, 10);
	FlowPanelSetBorder(row2, (Rectangle){ 10, 10, 10, 10 });

	ButtonCreate((Element*)row2, BUTTON_BORDER, "Button 4 in row", -1);
	ButtonCreate((Element*)row2, ELEMENT_H_FILL | BUTTON_BORDER, "Button 5 in row", -1);
	Button *button = ButtonCreate((Element*)row2, BUTTON_BORDER, "Button 6 in row", -1);
	ElementSetUserHandler((Element*)button, ButtonMessage);

	FlowPanel *row3 = FlowPanelCreate((Element*)column1, PANEL_BORDER | PANEL_HORIZONTAL | ELEMENT_H_FILL);
	FlowPanelSetGap(row2, 10);
	FlowPanelSetBorder(row2, (Rectangle){ 10, 10, 10, 10 });

	CheckboxCreate((Element*)row3, 0, "Checkmark", -1);
	CheckboxCreate((Element*)row3, CHECKBOX_CHECK_CROSS, "Cross", -1);

	return MessageLoop();
}

#endif
