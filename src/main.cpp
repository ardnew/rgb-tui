#include <fmt/core.h>
#include <cstddef>
#include <iostream>
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

using namespace ftxui;

Element ColorTile(int r, int g, int b) {
  return text(L"")                        //
         | size(WIDTH, EQUAL, 20)         //
         | size(HEIGHT, EQUAL, 7)         //
         | bgcolor(Color::RGB(r, g, b));  //
}

wchar_t HexLetter(int x) {
  if (x <= 9)
    return U'0' + x;
  else
    return U'A' + x - 9;
};

std::wstring HexColor(int r, int g, int b) {
  std::wstring out;
  out += L"#";
  out += HexLetter(r / 16);
  out += HexLetter(r % 16);
  out += HexLetter(g / 16);
  out += HexLetter(g % 16);
  out += HexLetter(b / 16);
  out += HexLetter(b % 16);
  return out;
}

Element HexaElement(int r, int g, int b) {
  return text(HexColor(r, g, b));
}

void ToRGB(int h,
           int s,
           int v,
           int& r,
           int& g,
           int& b) {
  if (s == 0) {
    r = v;
    g = v;
    b = v;
    return;
  }

  uint8_t region = h / 43;
  uint8_t remainder = (h - (region * 43)) * 6;
  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

  // clang-format off
  switch (region) {
    case 0: r = v, g = t, b = p; return;
    case 1: r = q, g = v, b = p; return;
    case 2: r = p, g = v, b = t; return;
    case 3: r = p, g = q, b = v; return;
    case 4: r = t, g = p, b = v; return;
    case 5: r = v, g = p, b = q; return;
  }
  // clang-format on
}

void ToHSV(int r, int g, int b, int& h, int& s, int& v) {
  int rgbMin = r < g ? (r < b ? r : b) : (g < b ? g : b);
  int rgbMax = r > g ? (r > b ? r : b) : (g > b ? g : b);

  v = rgbMax;
  if (v == 0) {
    h = 0;
    s = 0;
    return;
  }

  s = 255 * int(rgbMax - rgbMin) / v;
  if (s == 0) {
    h = 0;
    return;
  }

  if (rgbMax == r)
    h = 0 + 43 * (g - b) / (rgbMax - rgbMin);
  else if (rgbMax == g)
    h = 85 + 43 * (b - r) / (rgbMax - rgbMin);
  else
    h = 171 + 43 * (r - g) / (rgbMax - rgbMin);
}

class MainComponent : public ComponentBase {
 public:
  MainComponent(int& r, int& g, int& b) : r_(r), g_(g), b_(b) {
    Add(container_);
    ToHSV(r_, g_, b_, h_, s_, v_);
    box_color_.x_min = 0;
    box_color_.y_min = 0;
    box_color_.x_max = 80;
    box_color_.y_max = 1;
  }

 private:
  Element Render() final {
    std::string rgb_txt = fmt::format("{:3} , {:3} , {:3} ", r_, g_, b_);
    std::string hsv_txt = fmt::format("{:3}°, {:3}%, {:3}%",  //
                                      int(h_ * 360. / 255.),  //
                                      int(s_ * 100. / 255.),  //
                                      int(v_ * 100. / 255.)   //
    );

    int hue = h_;
    Elements array;
    int x_length = std::max(10, box_color_.x_max - box_color_.x_min) + 1;
    int y_length = 15;

    int h, s, v;
    ToHSV(r_, g_, b_, h, s, v);
    int target_x = std::max(0, std::min(x_length - 1, (v * x_length) / 255));
    int target_y =
        std::max(0, std::min(2 * y_length - 1, (s * 2 * y_length) / 255));

    for (int y = 0; y < y_length; ++y) {
      Elements line;
      for (int x = 0; x < x_length; ++x) {
        int saturation_1 = 255 * (y + 0.0f) / float(y_length);
        int saturation_2 = 255 * (y + 0.5f) / float(y_length);
        int value = 255 * x / float(x_length);
        if (x == target_x) {
          if (2 * y == target_y) {
            line.push_back(text(L"▀")                                     //
                           | color(Color::HSV(hue, saturation_1, value))  //
                           | bgcolor(Color::Black));                      //
            continue;
          }
          if (2 * y == target_y + 1) {
            line.push_back(text(L"▀")                                     //
                           | color(Color::Black)//
                           | bgcolor(Color::HSV(hue, saturation_2, value)));
            continue;
          }
        }
        line.push_back(text(L"▀")                                     //
                       | color(Color::HSV(hue, saturation_1, value))  //
                       | bgcolor(Color::HSV(hue, saturation_2, value)));
      }
      array.push_back(hbox(std::move(line)));
    }
    for (int saturation = 0; saturation < 255; saturation += 20) {
      Elements line;
      // for (int hue = 0; hue < 255; hue += 2) {
      array.push_back(hbox(std::move(line)));
    }

    return vbox({
               window(
                   text(L"[ rgb-tui ]") | center,  //
                   vbox({
                       hbox({
                           vbox(std::move(array)) | flex | reflect(box_color_),
                       }),
                       separator(),
                       hbox({
                           ColorTile(r_, g_, b_),
                           separator(),
                           vbox({
                               color_hue_->Render(),
                               color_saturation_->Render(),
                               color_value_->Render(),
                               separator(),
                               color_red_->Render(),
                               color_green_->Render(),
                               color_blue_->Render(),
                           }) | flex,
                       }),
                   })),
               hbox({
                   window(text(L" Hexa ") | center, HexaElement(r_, g_, b_)),
                   window(text(L" RGB ") | center, text(to_wstring(rgb_txt))),
                   window(text(L" HSV ") | center, text(to_wstring(hsv_txt))),
               }),
           }) |
           size(WIDTH, LESS_THAN, 80);
  };

  bool OnEvent(Event event) final {

    int r = r_;
    int g = g_;
    int b = b_;
    int h = h_;
    int s = s_;
    int v = v_;

    bool out = false;

    if (event.is_mouse())
      out |= OnMouseEvent(std::move(event));
    out |= ComponentBase::OnEvent(std::move(event));

    if (h != h_ || s != s_ || v != v_) {
      ToRGB(h_, s_, v_,  //
            r_, g_, b_);
    } else if (r != r_ || g != g_ || b != b_) {
      ToHSV(r_, g_, b_,  //
            h_, s_, v_);
    }
    return out;
  }

  bool OnMouseEvent(Event event) {
    if (event.mouse().motion == Mouse::Released) {
      captured_mouse_ = nullptr;
      return true;
    }

    if (box_color_.Contain(event.mouse().x, event.mouse().y) &&
        CaptureMouse(event)) {
      TakeFocus();
    }

    if (event.mouse().button == Mouse::Left &&
        event.mouse().motion == Mouse::Pressed &&
        box_color_.Contain(event.mouse().x, event.mouse().y) &&
        !captured_mouse_) {
      captured_mouse_ = CaptureMouse(event);
    }

    if (captured_mouse_) {
      v_ = (event.mouse().x - box_color_.x_min) * 255 /
           (box_color_.x_max - box_color_.x_min);
      s_ = (event.mouse().y - box_color_.y_min) * 255 /
           (box_color_.y_max - box_color_.y_min);
      v_ = std::max(0, std::min(255, v_));
      s_ = std::max(0, std::min(255, s_));
      return true;
    }
    return false;
  }

  int& r_;
  int& g_;
  int& b_;
  int h_;
  int s_;
  int v_;
  Component color_hue_ = Slider(L"Hue:        ", &h_, 0, 255, 1);
  Component color_saturation_ = Slider(L"Saturation: ", &s_, 0, 255, 1);
  Component color_value_ = Slider(L"Value:      ", &v_, 0, 255, 1);
  Component color_red_ = Slider(L"Red:        ", &r_, 0, 255, 1);
  Component color_green_ = Slider(L"Green:      ", &g_, 0, 255, 1);
  Component color_blue_ = Slider(L"Blue:       ", &b_, 0, 255, 1);
  Component container_ = Container::Vertical({
                                         color_hue_,
                                         color_saturation_,
                                         color_value_,
                                         color_red_,
                                         color_green_,
                                         color_blue_,
                                     });

  Box box_color_;
  CapturedMouse captured_mouse_;
};

int main(void) {
  int r = 255;
  int g = 0;
  int b = 0;
  auto screen = ScreenInteractive::TerminalOutput();
  screen.Loop(Make<MainComponent>(r, g, b));

  return EXIT_SUCCESS;
}
