#include <nvui/window.h>
#include <nvui/painter.h>
#include <nvui/widgets.h>
#include <nvui/menu.h>

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
		if(tristateCheck)
			CheckboxSetState(tristateCheck, Indeterminate);
	}
	return 0;
}

Textfield *tf;

int ButtonMessage2(Element *element, Message message, int di, void *dp)
{
	if(message == MSG_CLICKED)
	{
		TextfieldSetSelection(tf, 4, -1);
		WindowSetFocused(ElementGetWindow(element), E_OF(tf));
	}
	return 0;
}

int TextfieldMessage(Element *element, Message message, int di, void *dp)
{
	Textfield *textfield = (Textfield*)element;
	if(message == MSG_TEXTFIELD_TEXT_CHANGE)
	{
		printf("Text changed to: %s\n", TextfieldGetText(textfield, NULL));
	}
	return 0;
}

#if 0

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
	FlowPanelSetBorder(contentPanel, (Rectangle){ 10, 10, 10, 10 });

	FlowPanel *column = FlowPanelCreate((Element*)contentPanel, PANEL_BORDER | PANEL_BORDER_3D | ELEMENT_H_FILL | FLOWPANEL_HORIZONTAL);
	FlowPanelSetGap(column, 10);
	FlowPanelSetBorder(column, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate((Element*)column, 0, "Hello World", -1);
	Button *button = ButtonCreate((Element*)column, BUTTON_BORDER, "Test2", -1);
	ElementSetUserHandler((Element*)button, ButtonMessage);

	ButtonCreate((Element*)column, 0, "Test3", -1);

	Checkbox *checkbox = CheckboxCreate((Element*)column, 0, "Check", -1);
	ElementSetUserHandler((Element*)checkbox, CheckboxMessage);

	tristateCheck = CheckboxCreate((Element*)column, CHECKBOX_TRISTATE, "Tristate", -1);

	return MessageLoop();
}

#else

int main()
{
	Initialize();

	Menu *windowMenu = MenuCreate();
	MenuItem *fileMenu = MenuItemCreate("File", -1, 0);
	MenuItem *settingsMenu = MenuItemCreate("Settings", -1, 0);
	MenuAddItem(windowMenu, fileMenu);
	MenuAddItem(windowMenu, settingsMenu);

	Window *window1 = WindowCreate("Window 1", 1000, 800);
	if(!window1) return 1;
	WindowSetMenu(window1, windowMenu);

	FlowPanel *column1 = FlowPanelCreate(WindowGetRootElement(window1), 0);
	FlowPanelSetGap(column1, 10);
	FlowPanelSetBorder(column1, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate(E_OF(column1), 0, "Label 1", -1);
	LabelCreate(E_OF(column1), 0, "Longer label 2", -1);

	FlowPanel *column2 = FlowPanelCreate(E_OF(column1), PANEL_BORDER | PANEL_BORDER_3D);
	FlowPanelSetGap(column2, 10);
	FlowPanelSetBorder(column2, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate(E_OF(column2), ELEMENT_H_FILL, "Label 3", -1);
	LabelCreate(E_OF(column2), ELEMENT_H_FILL, "Much Longer label 4", -1);

	FlowPanel *column3 = FlowPanelCreate(E_OF(column1), PANEL_BORDER | PANEL_BORDER_3D | ELEMENT_H_FILL);
	FlowPanelSetGap(column2, 10);
	FlowPanelSetBorder(column2, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate(E_OF(column3), 0, "Label 4", -1);
	LabelCreate(E_OF(column3), 0, "Longer label 5", -1);

	FlowPanel *column4 = FlowPanelCreate(E_OF(column1), PANEL_BORDER | ELEMENT_H_FILL);
	FlowPanelSetGap(column4, 10);
	FlowPanelSetBorder(column4, (Rectangle){ 10, 10, 10, 10 });

	LabelCreate(E_OF(column4), ELEMENT_H_FILL, "Label 6", -1);
	LabelCreate(E_OF(column4), ELEMENT_H_FILL, "Longer label 7", -1);

	ButtonCreate(E_OF(column1), ELEMENT_V_FILL, "Vertical fill button 1", -1);
	ButtonCreate(E_OF(column1), ELEMENT_V_FILL, "Vertical fill button 2", -1);
	ButtonCreate(E_OF(column1), ELEMENT_V_FILL | ELEMENT_H_FILL, "Vertical and horizontal fill button 3", -1);
	ButtonCreate(E_OF(column1), ELEMENT_V_FILL, "Vertical fill button 4", -1);

	FlowPanel *row1 = FlowPanelCreate(E_OF(column1), PANEL_BORDER | FLOWPANEL_HORIZONTAL);
	FlowPanelSetGap(row1, 10);
	FlowPanelSetBorder(row1, (Rectangle){ 10, 10, 10, 10 });

	ButtonCreate(E_OF(row1), 0, "Button 1 in row", -1);
	ButtonCreate(E_OF(row1), ELEMENT_H_FILL, "Button 2 in row", -1);
	ButtonCreate(E_OF(row1), 0, "Button 3 in row", -1);

	FlowPanel *row2 = FlowPanelCreate(E_OF(column1), PANEL_BORDER | FLOWPANEL_HORIZONTAL | ELEMENT_H_FILL);
	FlowPanelSetGap(row2, 10);
	FlowPanelSetBorder(row2, (Rectangle){ 10, 10, 10, 10 });

	ButtonCreate(E_OF(row2), BUTTON_BORDER, "Button 4 in row", -1);
	Button *button = ButtonCreate(E_OF(row2), ELEMENT_H_FILL | BUTTON_BORDER, "Button 5 in row", -1);
	ElementSetUserHandler(E_OF(button), ButtonMessage2);
	button = ButtonCreate(E_OF(row2), BUTTON_BORDER, "Button 6 in row", -1);
	ElementSetUserHandler(E_OF(button), ButtonMessage);

	FlowPanel *row3 = FlowPanelCreate(E_OF(column1), PANEL_BORDER | FLOWPANEL_HORIZONTAL | ELEMENT_H_FILL);
	FlowPanelSetGap(row3, 10);
	FlowPanelSetBorder(row3, (Rectangle){ 10, 10, 10, 10 });

	CheckboxCreate(E_OF(row3), CHECKBOX_BORDER, "Checkmark", -1);
	CheckboxCreate(E_OF(row3), CHECKBOX_CHECK_CROSS, "Cross", -1);
	Textfield *textfield = TextfieldCreate(E_OF(row3), 0, 7);
	ElementSetUserHandler(E_OF(textfield), TextfieldMessage);
	Textfield *textfield2 = TextfieldCreate(E_OF(row3), ELEMENT_H_FILL, 255);
	TextfieldSetText(textfield2, "HelloWorld!!!", -1);

	tf = textfield2;

	return MessageLoop();
}

#endif
