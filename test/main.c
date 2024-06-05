#include <nvui/window.h>
#include <nvui/painter.h>
#include <stdio.h>
#include <string.h>

int MyElementMessage(Element *element, Message message, int di, void *dp) {
	(void) di;

	if (message == MSG_PAINT) {
		Painter *painter = dp;
		painter->backColor = ColorFromInt(0xFFCCFF);
		PainterFillRect(painter, element->bounds);

		for (int i = 0; i < 5; i++) {
			for (int j = 0; j < 5; j++) {
				painter->backColor = ColorFromInt(0xFFFFFF);
				PainterFillRect(painter, (Rectangle){20 + j * 30, 40 + j * 30, 20 + i * 30, 40 + i * 30});
				painter->backColor = ColorFromInt(0x000000);
				PainterDrawRect(painter, (Rectangle){20 + j * 30, 40 + j * 30, 20 + i * 30, 40 + i * 30});
			}
		}

		const char *message = "Hello World!";

		for (int i = -2; i <= 2; i++) {
			for (int j = -2; j <= 2; j++) {
				Rectangle rectangle = (Rectangle){element->bounds.l - j, element->bounds.r - j, 
						element->bounds.t - i, element->bounds.b - i};
				painter->backColor = ColorFromInt(0xffffff);
				painter->fontStyle = Regular;
				PainterDrawString(painter, rectangle, message, strlen(message), true);
			}
		}

		painter->fontStyle = Regular;
		painter->backColor = ColorFromInt(0x000000);
		PainterDrawString(painter, element->bounds, message, strlen(message), true);
	} else if (message == MSG_LAYOUT) {
		fprintf(stderr, "layout with bounds (%d->%d;%d->%d)\n", element->bounds.l, element->bounds.r, element->bounds.t, element->bounds.b);
	}

	return 0;
}

int main() {
	Initialize();
	Window *window = WindowCreate("Hello, world", 640, 480);
	ElementCreate(sizeof(Element), &window->e, 0, MyElementMessage);
	return MessageLoop();
}
