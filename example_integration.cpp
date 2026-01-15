/**
 * Example: Integrating SyslogDialog into a tabbed application
 *
 * This demonstrates how to use the SyslogDialog widget in a
 * multi-tab application interface.
 *
 * Build:
 *   g++ -o tabbed_app example_integration.cpp \
 *       -I./include \
 *       -L./build \
 *       -lsyslog_dialog_lib \
 *       $(pkg-config --cflags --libs gtkmm-3.0) \
 *       -lpthread
 */

#include <gtkmm.h>

#include "syslog_dialog.h"

// Example placeholder widget for other tabs
class DashboardWidget : public Gtk::Box {
 public:
  DashboardWidget() : Gtk::Box(Gtk::ORIENTATION_VERTICAL) {
    label_.set_text("Dashboard Content Goes Here");
    label_.set_margin_top(20);
    label_.set_margin_bottom(20);
    pack_start(label_, Gtk::PACK_EXPAND_WIDGET);
    show_all_children();
  }

 private:
  Gtk::Label label_;
};

class NetworkWidget : public Gtk::Box {
 public:
  NetworkWidget() : Gtk::Box(Gtk::ORIENTATION_VERTICAL) {
    label_.set_text("Network Monitoring Content Goes Here");
    label_.set_margin_top(20);
    label_.set_margin_bottom(20);
    pack_start(label_, Gtk::PACK_EXPAND_WIDGET);
    show_all_children();
  }

 private:
  Gtk::Label label_;
};

class TabbedApplication : public Gtk::Window {
 public:
  TabbedApplication() {
    set_title("Multi-Tab System Monitor");
    set_default_size(1200, 700);
    set_border_width(0);

    // Create menu bar
    create_menubar();

    // Create main container
    main_box_.set_orientation(Gtk::ORIENTATION_VERTICAL);

    // Add menu bar to main box
    main_box_.pack_start(menubar_, Gtk::PACK_SHRINK);

    // Create tabs
    notebook_.set_tab_pos(Gtk::POS_TOP);

    // Add Dashboard tab
    notebook_.append_page(dashboard_widget_, "Dashboard");

    // Add Syslog tab with the reusable SyslogDialog
    notebook_.append_page(syslog_dialog_, "Syslog Viewer");

    // Add Network tab
    notebook_.append_page(network_widget_, "Network");

    // Add notebook to main box
    main_box_.pack_start(notebook_, Gtk::PACK_EXPAND_WIDGET);

    // Add status bar
    statusbar_.push("Ready");
    main_box_.pack_start(statusbar_, Gtk::PACK_SHRINK);

    add(main_box_);
    show_all_children();

    // Connect tab change signal
    notebook_.signal_switch_page().connect(
        sigc::mem_fun(*this, &TabbedApplication::on_tab_changed));
  }

  ~TabbedApplication() override = default;

 private:
  Gtk::Box main_box_;
  Gtk::MenuBar menubar_;
  Gtk::Notebook notebook_;
  Gtk::Statusbar statusbar_;

  // Tab widgets
  DashboardWidget dashboard_widget_;
  SyslogDialog syslog_dialog_;
  NetworkWidget network_widget_;

  void create_menubar() {
    // File menu
    auto file_menu = Gtk::manage(new Gtk::Menu());
    auto file_item = Gtk::manage(new Gtk::MenuItem("_File", true));
    file_item->set_submenu(*file_menu);

    auto quit_item = Gtk::manage(new Gtk::MenuItem("_Quit", true));
    quit_item->signal_activate().connect(
        sigc::mem_fun(*this, &TabbedApplication::on_quit));
    file_menu->append(*quit_item);

    menubar_.append(*file_item);

    // View menu
    auto view_menu = Gtk::manage(new Gtk::Menu());
    auto view_item = Gtk::manage(new Gtk::MenuItem("_View", true));
    view_item->set_submenu(*view_menu);

    auto dashboard_item = Gtk::manage(new Gtk::MenuItem("_Dashboard", true));
    dashboard_item->signal_activate().connect(
        [this]() { notebook_.set_current_page(0); });
    view_menu->append(*dashboard_item);

    auto syslog_item = Gtk::manage(new Gtk::MenuItem("_Syslog", true));
    syslog_item->signal_activate().connect(
        [this]() { notebook_.set_current_page(1); });
    view_menu->append(*syslog_item);

    auto network_item = Gtk::manage(new Gtk::MenuItem("_Network", true));
    network_item->signal_activate().connect(
        [this]() { notebook_.set_current_page(2); });
    view_menu->append(*network_item);

    menubar_.append(*view_item);

    // Syslog menu
    auto syslog_menu = Gtk::manage(new Gtk::Menu());
    auto syslog_menu_item = Gtk::manage(new Gtk::MenuItem("_Syslog", true));
    syslog_menu_item->set_submenu(*syslog_menu);

    auto start_item = Gtk::manage(new Gtk::MenuItem("_Start Listening", true));
    start_item->signal_activate().connect(
        [this]() { syslog_dialog_.start_listening(); });
    syslog_menu->append(*start_item);

    auto stop_item = Gtk::manage(new Gtk::MenuItem("S_top Listening", true));
    stop_item->signal_activate().connect(
        [this]() { syslog_dialog_.stop_listening(); });
    syslog_menu->append(*stop_item);

    auto clear_item = Gtk::manage(new Gtk::MenuItem("_Clear Messages", true));
    clear_item->signal_activate().connect(
        [this]() { syslog_dialog_.clear_messages(); });
    syslog_menu->append(*clear_item);

    menubar_.append(*syslog_menu_item);

    menubar_.show_all();
  }

  void on_tab_changed(Gtk::Widget* page, guint page_num) {
    switch (page_num) {
      case 0:
        statusbar_.pop();
        statusbar_.push("Dashboard view active");
        break;
      case 1:
        statusbar_.pop();
        statusbar_.push("Syslog viewer active - Port: " +
                        std::to_string(syslog_dialog_.get_port()));
        break;
      case 2:
        statusbar_.pop();
        statusbar_.push("Network monitor active");
        break;
    }
  }

  void on_quit() { hide(); }
};

int main(int argc, char* argv[]) {
  auto app = Gtk::Application::create(argc, argv, "com.example.tabbed.monitor");

  TabbedApplication window;

  return app->run(window);
}
