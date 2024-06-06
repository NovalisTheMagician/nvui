#include <nvui/window.h>
#include <nvui/painter.h>
#include <stdio.h>
#include <string.h>

Element *parentElement, *childElement;

int ParentElementMessage(Element *element, Message message, int di, void *dp) {
	if (message == MSG_PAINT) {
		Painter *painter = dp;
		painter->backColor = ColorFromInt(0xFFCCFF);
		PainterFillRect(painter, element->bounds);
	} else if (message == MSG_LAYOUT) {
		fprintf(stderr, "layout with bounds (%d->%d;%d->%d)\n", element->bounds.l, element->bounds.r, element->bounds.t, element->bounds.b);
		ElementMove(childElement, (Rectangle){50, 100, 50, 100}, false);
	} else if (message == MSG_MOUSE_MOVE) {
		fprintf(stderr, "mouse move at (%d,%d)\n", element->window->cursorX, element->window->cursorY);
	} else if (message == MSG_MOUSE_DRAG) {
		fprintf(stderr, "mouse drag at (%d,%d)\n", element->window->cursorX, element->window->cursorY);
	} else if (message == MSG_UPDATE) {
		fprintf(stderr, "update %d\n", di);
	} else if (message == MSG_LEFT_DOWN) {
		fprintf(stderr, "left down\n");
	} else if (message == MSG_RIGHT_DOWN) {
		fprintf(stderr, "right down\n");
	} else if (message == MSG_MIDDLE_DOWN) {
		fprintf(stderr, "middle down\n");
	} else if (message == MSG_LEFT_UP) {
		fprintf(stderr, "left up\n");
	} else if (message == MSG_RIGHT_UP) {
		fprintf(stderr, "right up\n");
	} else if (message == MSG_MIDDLE_UP) {
		fprintf(stderr, "middle up\n");
	} else if (message == MSG_CLICKED) {
		fprintf(stderr, "clicked\n");
	}

	return 0;
}

int ChildElementMessage(Element *element, Message message, int di, void *dp) {
	(void) di;

	if (message == MSG_PAINT) {
		Painter *painter = dp;
		painter->backColor = ColorFromInt(0x444444);
		PainterFillRect(painter, element->bounds);
	}

	return 0;
}

int main() {
	Initialize();
	Window *window = WindowCreate("Hello, world", 300, 200);
	parentElement = ElementCreate(sizeof(Element), &window->e, 0, ParentElementMessage);
	childElement = ElementCreate(sizeof(Element), parentElement, 0, ChildElementMessage);
	return MessageLoop();
}
