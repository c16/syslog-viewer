#include <gtkmm.h>

#include "syslog_dialog.h"

class SyslogWindow : public Gtk::Window {
 public:
  SyslogWindow() {
    set_title("Syslog Viewer");
    set_default_size(1200, 600);
    set_border_width(0);

    add(syslog_dialog_);
    show_all_children();
  }

  ~SyslogWindow() override = default;

 private:
  SyslogDialog syslog_dialog_;
};

int main(int argc, char* argv[]) {
  auto app = Gtk::Application::create(argc, argv, "com.syslog.viewer");

  SyslogWindow window;

  return app->run(window);
}
