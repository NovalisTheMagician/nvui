#include <nvui/window.h>
#include <nvui/painter.h>
#include <nvui/widgets.h>
#include <stdio.h>
#include <string.h>

Button *myButton;
Label *myLabel;
int counter = 0;

void UpdateLabel(void) {
	char buffer[50];
	snprintf(buffer, sizeof(buffer), "Click count: %d", counter);
	LabelSetContent(myLabel, buffer, -1);
	ElementRepaint(&myLabel->e, NULL);
}

int LayoutElementMessage(Element *element, Message message, int di, void *dp) {
	(void) di;

	if (message == MSG_LAYOUT) {
		ElementMove(&myButton->e, (Rectangle){10, 200, 10, 40}, false);
		ElementMove(&myLabel->e, (Rectangle){10, element->bounds.r - 10, 50, 90}, false);
	} else if (message == MSG_PAINT) {
		Painter *painter = dp;
		painter->backColor = ColorFromInt(0xFFCCFF);
		PainterFillRect(painter, element->bounds);
	}

	return 0;
}

int MyButtonMessage(Element *element, Message message, int di, void *dp) {
	(void) element;
	(void) di;
	(void) dp;

	if (message == MSG_CLICKED) {
		counter++;
		UpdateLabel();
	}
	
	return 0;
}

int main() {
	Initialize();
	Window *window = WindowCreate("Hello, world", 300, 200);
	Element *layoutElement = ElementCreate(sizeof(Element), &window->e, 0, LayoutElementMessage);
	myButton = ButtonCreate(layoutElement, 0, "Increment counter", -1);
	myButton->e.messageUser = MyButtonMessage;
	myLabel = LabelCreate(layoutElement, LABEL_CENTER, NULL, 0);
	UpdateLabel();
	return MessageLoop();
}
