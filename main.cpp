#include "main.h"
#include "json11.hpp"

#define IMAGE_SIZE         48
#define RECT_SIZE          15
#define MARGIN              8

using namespace std;
using namespace json11;

#define SSTR(x) static_cast<std::ostringstream &>(           \
                    (std::ostringstream() << std::dec << x)) \
                    .str()

Display7Segments* dis7;
char request_body[256];
bool request = false;

void secondThread() {
    while(true) {
        char* line = getLine(false, '~');
        snprintf(request_body, 256, "%s",line);
        free(line);
        request = true;
    }
}

uint8_t lerp(int start, int end, float porcent) {
    return start + (end - start) * porcent;
}

int main() {
    stdio_init_all();
    dis7 = new Display7Segments();
    multicore_launch_core1(secondThread);
    uint8_t counter = 0;
    float time = 0.0f, delta = 1 / 50.0f;
    sleep_ms(250);
    SSD1306Driver* display = new SSD1306Driver(16, 17, 0, 0x3C, Size::SIZE_128x64);
    display->orientation(0);
    Button* btn1 = new Button(10);
    Button* btn2 = new Button(9);
    Button* btn3 = new Button(8);
    Button* btn4 = new Button(7);
    Button* btn5 = new Button(6);
    LED* led = new LED(18);
    char input[256];
    uint8_t position = 0;
    std::vector<string>* options = new std::vector<string>();
    options->push_back("Luz");
    options->push_back("Crono");
    options->push_back("Android");
    options->push_back("Rate");
    uint RIGHT_POS_X = 128 - (MARGIN + RECT_SIZE), RIGHT_POS_Y = 32 - (RECT_SIZE / 2);
    uint LEFT_POS_X = MARGIN, LEFT_POS_Y = 32 - (RECT_SIZE / 2);
    uint CENTER_POS_X = 64 - (IMAGE_SIZE / 2), CENTER_POS_Y = 32 - (IMAGE_SIZE / 2);
    bool next = false, show_left = false, show_right = options->size() > 1;
    uint8_t state = 0;
    while (true) {
        if (btn1->isPressed())
        {
            if (position - 1 >= 0)
            {
                position--;
                state = 1;
                next = false;
                show_left = position > 0;
            }
            sleep_ms(200);
        }
        else if (btn2->isPressed())
        {
            if (position + 1 < options->size())
            {
                position++;
                state = 1;
                next = true;
                show_right = position < options->size() - 1;
            }
            sleep_ms(200);
        }
        else if (btn3->isPressed())
        {
            led->setState(!led->isOn());
            sleep_ms(200);
        }
        else if (btn4->isPressed())
        {
            dis7->setCounter(counter++);
            sleep_ms(200);
        }

        if(request) {
            string err;
            Json data = Json::parse(request_body, err);
            snprintf(input, 256, "%s", data["text"].string_value().c_str());
            led->setState(data["led"].bool_value());
            request = false;
        }
        display->clear();
        display->text(input, 0, 0);
        if(show_left) {
            display->rect(LEFT_POS_X, LEFT_POS_Y, RECT_SIZE, RECT_SIZE, false);
        }

        if (show_right) {
            display->rect(RIGHT_POS_X, RIGHT_POS_Y, RECT_SIZE, RECT_SIZE, false);
        }

        if(state == 1) {
            float porcent = time / 0.4f;
            float porcent_ = next ? porcent : (1 - porcent);
            display->rect(
                lerp(CENTER_POS_X, LEFT_POS_X, porcent_),
                lerp(CENTER_POS_Y, LEFT_POS_Y, porcent_),
                lerp(IMAGE_SIZE, RECT_SIZE, porcent_),
                lerp(IMAGE_SIZE, RECT_SIZE, porcent_), false);
            display->rect(
                lerp(RIGHT_POS_X, CENTER_POS_X, porcent_),
                lerp(RIGHT_POS_Y, CENTER_POS_Y, porcent_),
                lerp(RECT_SIZE, IMAGE_SIZE, porcent_),
                lerp(RECT_SIZE, IMAGE_SIZE, porcent_), false);
            if (time + delta > 0.4f) {
                time = 0;
                state = 0;
                show_left = position > 0;
                show_right = position < options->size() - 1;
                snprintf(input, 256, "%s", options->at(position).c_str());
            }
            time += delta;
        } else {
            uint8_t* img = NULL;
            switch (position)
            {
            case 0:
                img = led_img;
                break;
            case 1:
                img = time_img;
                break;
            case 2:
                img = android_img;
                break;
            case 3:
                img = test_img;
                break;
            default:
                break;
            }
            display->rect(CENTER_POS_X,CENTER_POS_Y, IMAGE_SIZE, IMAGE_SIZE, false);
            display->image(CENTER_POS_X,CENTER_POS_Y, IMAGE_SIZE, IMAGE_SIZE, img, WriteMode::Invert);
            
        }
        display->sendBuffer();
        dis7->update();
    }
}