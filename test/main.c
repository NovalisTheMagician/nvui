#include <nvui/window.h>
#include <nvui/painter.h>
#include <stdio.h>

Element *elementA, *elementB, *elementC, *elementD;

int ElementAMessage(Element *element, Message message, int di, void *dp) {
	(void) di;
    

	Rectangle bounds = element->bounds;

	if (message == MSG_PAINT) {
        Painter *painter = dp;
        painter->backColor = ColorFromInt(0xFF77FF);
		//PainterFillRect(painter, bounds);
        PainterClear(painter);
	} else if (message == MSG_LAYOUT) {
		//fprintf(stderr, "layout A with bounds (%d->%d;%d->%d)\n", bounds.l, bounds.r, bounds.t, bounds.b);
		ElementMove(elementB, (Rectangle){bounds.l + 20, bounds.r - 20, bounds.t + 20, bounds.b - 20}, false);
	}

	return 0;
}

int ElementBMessage(Element *element, Message message, int di, void *dp) {
	(void) di;

	Rectangle bounds = element->bounds;

	if (message == MSG_PAINT) {
        Painter *painter = dp;
        painter->backColor = ColorFromInt(0xDDDDE0);
		//PainterFillRect(painter, bounds);
        PainterClear(painter);
	} else if (message == MSG_LAYOUT) {
		//fprintf(stderr, "layout B with bounds (%d->%d;%d->%d)\n", bounds.l, bounds.r, bounds.t, bounds.b);
		ElementMove(elementC, (Rectangle){bounds.l - 40, bounds.l + 40, bounds.t + 40, bounds.b - 40}, false);
		ElementMove(elementD, (Rectangle){bounds.r - 40, bounds.r + 40, bounds.t + 40, bounds.b - 40}, false);
	}

	return 0;
}

int ElementCMessage(Element *element, Message message, int di, void *dp) {
	(void) di;

	Rectangle bounds = element->bounds;

	if (message == MSG_PAINT) {
        Painter *painter = dp;
        painter->backColor = ColorFromInt(0x3377FF);
		//PainterFillRect(painter, bounds);
        PainterClear(painter);
	} else if (message == MSG_LAYOUT) {
		//fprintf(stderr, "layout C with bounds (%d->%d;%d->%d)\n", bounds.l, bounds.r, bounds.t, bounds.b);
	}

	return 0;
}

int ElementDMessage(Element *element, Message message, int di, void *dp) {
	(void) di;

	Rectangle bounds = element->bounds;

	if (message == MSG_PAINT) {
        Painter *painter = dp;
        painter->backColor = ColorFromInt(0x33CC33);
		//PainterFillRect(painter, bounds);
        PainterClear(painter);
	} else if (message == MSG_LAYOUT) {
		//fprintf(stderr, "layout D with bounds (%d->%d;%d->%d)\n", bounds.l, bounds.r, bounds.t, bounds.b);
	}

	return 0;
}

int main(int argc, char *argv[])
{
    Initialize();
    Window *window = WindowCreate("Hello World!", 800, 600);
	if(!window)
	{
		printf("failed to create window\n");
		return 1;
	}
    elementA = ElementCreate(sizeof(Element), &window->e, 0, ElementAMessage);
	elementB = ElementCreate(sizeof(Element), elementA, 0, ElementBMessage);
	elementC = ElementCreate(sizeof(Element), elementB, 0, ElementCMessage);
	elementD = ElementCreate(sizeof(Element), elementB, 0, ElementDMessage);
    return MessageLoop();
}
