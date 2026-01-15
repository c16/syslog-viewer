#ifndef SYSLOG_DIALOG_H
#define SYSLOG_DIALOG_H

#include <gtkmm.h>

#include <fstream>
#include <memory>
#include <mutex>
#include <vector>

#include "syslog_message.h"
#include "udp_listener.h"

class SyslogDialog : public Gtk::Box {
 public:
  SyslogDialog();
  virtual ~SyslogDialog();

  // Start/stop listening
  void start_listening();
  void stop_listening();
  bool is_listening() const;

  // Port configuration
  void set_port(int port);
  int get_port() const;

  // Clear messages
  void clear_messages();

  // Export messages
  void export_to_file(const std::string& filename);

  // File logging
  void enable_file_logging(bool enabled);
  bool is_file_logging_enabled() const;
  void set_log_file_path(const std::string& path);
  std::string get_log_file_path() const;

 protected:
  // UI Components
  Gtk::Box controls_box_;
  Gtk::Box filter_box_;
  Gtk::Box button_box_;
  Gtk::Box logging_box_;

  Gtk::Label port_label_;
  Gtk::SpinButton port_spin_;
  Gtk::Button start_button_;
  Gtk::Button stop_button_;
  Gtk::Button clear_button_;
  Gtk::Button export_button_;

  Gtk::CheckButton enable_logging_check_;
  Gtk::Label log_file_label_;
  Gtk::Entry log_file_entry_;
  Gtk::Button log_file_browse_button_;

  Gtk::Label filter_label_;
  Gtk::Entry filter_entry_;
  Gtk::CheckButton emergency_check_;
  Gtk::CheckButton alert_check_;
  Gtk::CheckButton critical_check_;
  Gtk::CheckButton error_check_;
  Gtk::CheckButton warning_check_;
  Gtk::CheckButton notice_check_;
  Gtk::CheckButton info_check_;
  Gtk::CheckButton debug_check_;

  Gtk::ScrolledWindow scrolled_window_;
  Gtk::TreeView tree_view_;

  Gtk::Label status_label_;

  // Tree model columns
  class ModelColumns : public Gtk::TreeModel::ColumnRecord {
   public:
    ModelColumns() {
      add(timestamp);
      add(severity);
      add(facility);
      add(hostname);
      add(application);
      add(message);
      add(source_ip);
      add(severity_enum);
    }

    Gtk::TreeModelColumn<Glib::ustring> timestamp;
    Gtk::TreeModelColumn<Glib::ustring> severity;
    Gtk::TreeModelColumn<Glib::ustring> facility;
    Gtk::TreeModelColumn<Glib::ustring> hostname;
    Gtk::TreeModelColumn<Glib::ustring> application;
    Gtk::TreeModelColumn<Glib::ustring> message;
    Gtk::TreeModelColumn<Glib::ustring> source_ip;
    Gtk::TreeModelColumn<int> severity_enum;  // Hidden, for filtering
  };

  ModelColumns columns_;
  Glib::RefPtr<Gtk::ListStore> tree_model_;
  Glib::RefPtr<Gtk::TreeModelFilter> filtered_model_;

  // UDP Listener
  std::unique_ptr<UdpListener> listener_;

  // Message storage
  std::vector<SyslogMessage> messages_;
  std::mutex messages_mutex_;

  // Signal handlers
  void on_start_clicked();
  void on_stop_clicked();
  void on_clear_clicked();
  void on_export_clicked();
  void on_filter_changed();
  void on_logging_toggled();
  void on_log_file_browse_clicked();

  // Message handling
  void on_message_received(const SyslogMessage& msg);
  void add_message_to_view(const SyslogMessage& msg);
  void on_message_dispatch();  // No-argument wrapper for dispatcher

  // Filtering
  bool filter_func(const Gtk::TreeModel::const_iterator& iter);

  // Update status
  void update_status();

  // Dispatcher for thread-safe UI updates
  Glib::Dispatcher message_dispatcher_;
  SyslogMessage pending_message_;
  std::mutex pending_mutex_;

  // File logging
  std::ofstream log_file_stream_;
  std::string log_file_path_;
  bool file_logging_enabled_;
  std::mutex log_file_mutex_;
  void write_message_to_file(const SyslogMessage& msg);

  // Config persistence
  void load_config();
  void save_config();
  std::string get_config_path() const;
  bool loading_config_;
};

#endif  // SYSLOG_DIALOG_H
