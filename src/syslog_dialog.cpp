#include "syslog_dialog.h"

#include <fstream>
#include <iostream>

SyslogDialog::SyslogDialog()
    : Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5),
      controls_box_(Gtk::ORIENTATION_HORIZONTAL, 5),
      filter_box_(Gtk::ORIENTATION_HORIZONTAL, 5),
      button_box_(Gtk::ORIENTATION_HORIZONTAL, 5),
      logging_box_(Gtk::ORIENTATION_HORIZONTAL, 5),
      port_label_("Port:"),
      start_button_("Start"),
      stop_button_("Stop"),
      clear_button_("Clear"),
      export_button_("Export"),
      enable_logging_check_("Log to file"),
      log_file_label_("Log file:"),
      log_file_browse_button_("Browse..."),
      filter_label_("Filter:"),
      emergency_check_("EMERG"),
      alert_check_("ALERT"),
      critical_check_("CRIT"),
      error_check_("ERROR"),
      warning_check_("WARN"),
      notice_check_("NOTICE"),
      info_check_("INFO"),
      debug_check_("DEBUG"),
      status_label_("Not listening"),
      listener_(std::make_unique<UdpListener>(514)),
      file_logging_enabled_(false),
      loading_config_(false) {
  set_margin_top(5);
  set_margin_bottom(5);
  set_margin_start(5);
  set_margin_end(5);

  // Configure port spin button
  port_spin_.set_range(1, 65535);
  port_spin_.set_value(514);
  port_spin_.set_increments(1, 10);
  port_spin_.set_digits(0);

  // Set up controls box
  controls_box_.set_margin_bottom(5);
  controls_box_.pack_start(port_label_, Gtk::PACK_SHRINK);
  controls_box_.pack_start(port_spin_, Gtk::PACK_SHRINK);
  controls_box_.pack_start(start_button_, Gtk::PACK_SHRINK);
  controls_box_.pack_start(stop_button_, Gtk::PACK_SHRINK);
  controls_box_.pack_start(clear_button_, Gtk::PACK_SHRINK);
  controls_box_.pack_start(export_button_, Gtk::PACK_SHRINK);
  controls_box_.pack_start(status_label_, Gtk::PACK_EXPAND_WIDGET);

  // Set up logging box
  logging_box_.set_margin_bottom(5);
  logging_box_.pack_start(enable_logging_check_, Gtk::PACK_SHRINK);
  logging_box_.pack_start(log_file_label_, Gtk::PACK_SHRINK);
  logging_box_.pack_start(log_file_entry_, Gtk::PACK_EXPAND_WIDGET);
  logging_box_.pack_start(log_file_browse_button_, Gtk::PACK_SHRINK);

  // Set default log file path
  log_file_entry_.set_text("/tmp/syslog_viewer.log");
  log_file_path_ = "/tmp/syslog_viewer.log";

  // Set up filter box
  filter_box_.set_margin_bottom(5);
  filter_box_.pack_start(filter_label_, Gtk::PACK_SHRINK);
  filter_box_.pack_start(filter_entry_, Gtk::PACK_EXPAND_WIDGET);

  // Set up severity filter checkboxes
  emergency_check_.set_active(true);
  alert_check_.set_active(true);
  critical_check_.set_active(true);
  error_check_.set_active(true);
  warning_check_.set_active(true);
  notice_check_.set_active(true);
  info_check_.set_active(true);
  debug_check_.set_active(true);

  button_box_.set_margin_bottom(5);
  button_box_.pack_start(emergency_check_, Gtk::PACK_SHRINK);
  button_box_.pack_start(alert_check_, Gtk::PACK_SHRINK);
  button_box_.pack_start(critical_check_, Gtk::PACK_SHRINK);
  button_box_.pack_start(error_check_, Gtk::PACK_SHRINK);
  button_box_.pack_start(warning_check_, Gtk::PACK_SHRINK);
  button_box_.pack_start(notice_check_, Gtk::PACK_SHRINK);
  button_box_.pack_start(info_check_, Gtk::PACK_SHRINK);
  button_box_.pack_start(debug_check_, Gtk::PACK_SHRINK);

  // Create tree model
  tree_model_ = Gtk::ListStore::create(columns_);
  filtered_model_ = Gtk::TreeModelFilter::create(tree_model_);
  filtered_model_->set_visible_func(
      sigc::mem_fun(*this, &SyslogDialog::filter_func));

  tree_view_.set_model(filtered_model_);

  // Add columns
  tree_view_.append_column("Timestamp", columns_.timestamp);
  tree_view_.append_column("Severity", columns_.severity);
  tree_view_.append_column("Facility", columns_.facility);
  tree_view_.append_column("Source IP", columns_.source_ip);
  tree_view_.append_column("Hostname", columns_.hostname);
  tree_view_.append_column("Application", columns_.application);
  tree_view_.append_column("Message", columns_.message);

  // Make columns resizable and sortable
  for (auto column : tree_view_.get_columns()) {
    column->set_resizable(true);
    column->set_sort_column(column->get_sort_column_id());
  }

  // Set up scrolled window
  scrolled_window_.add(tree_view_);
  scrolled_window_.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

  // Pack everything
  pack_start(controls_box_, Gtk::PACK_SHRINK);
  pack_start(logging_box_, Gtk::PACK_SHRINK);
  pack_start(filter_box_, Gtk::PACK_SHRINK);
  pack_start(button_box_, Gtk::PACK_SHRINK);
  pack_start(scrolled_window_, Gtk::PACK_EXPAND_WIDGET);

  // Connect signals
  start_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_start_clicked));
  stop_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_stop_clicked));
  clear_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_clear_clicked));
  export_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_export_clicked));

  filter_entry_.signal_changed().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_filter_changed));

  emergency_check_.signal_toggled().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_filter_changed));
  alert_check_.signal_toggled().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_filter_changed));
  critical_check_.signal_toggled().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_filter_changed));
  error_check_.signal_toggled().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_filter_changed));
  warning_check_.signal_toggled().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_filter_changed));
  notice_check_.signal_toggled().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_filter_changed));
  info_check_.signal_toggled().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_filter_changed));
  debug_check_.signal_toggled().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_filter_changed));

  enable_logging_check_.signal_toggled().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_logging_toggled));

  log_file_browse_button_.signal_clicked().connect(
      sigc::mem_fun(*this, &SyslogDialog::on_log_file_browse_clicked));

  // Set up message dispatcher
  message_dispatcher_.connect(
      sigc::mem_fun(*this, &SyslogDialog::on_message_dispatch));

  // Initial button states
  stop_button_.set_sensitive(false);
  update_status();

  // Load saved configuration
  load_config();

  show_all_children();
}

SyslogDialog::~SyslogDialog() {
  save_config();
  stop_listening();
  if (log_file_stream_.is_open()) {
    log_file_stream_.close();
  }
}

void SyslogDialog::start_listening() {
  if (listener_->is_listening()) {
    return;
  }

  int port = static_cast<int>(port_spin_.get_value());
  listener_->set_port(port);

  try {
    listener_->start(
        [this](const SyslogMessage& msg) { on_message_received(msg); });

    // Save the port configuration
    save_config();

    start_button_.set_sensitive(false);
    stop_button_.set_sensitive(true);
    port_spin_.set_sensitive(false);
    update_status();
  } catch (const std::exception& e) {
    Gtk::MessageDialog dialog(
        "Error starting listener: " + std::string(e.what()), false,
        Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.run();
  }
}

void SyslogDialog::stop_listening() {
  listener_->stop();
  start_button_.set_sensitive(true);
  stop_button_.set_sensitive(false);
  port_spin_.set_sensitive(true);
  update_status();
}

bool SyslogDialog::is_listening() const { return listener_->is_listening(); }

void SyslogDialog::set_port(int port) {
  if (!listener_->is_listening()) {
    port_spin_.set_value(port);
    listener_->set_port(port);
  }
}

int SyslogDialog::get_port() const { return listener_->get_port(); }

void SyslogDialog::clear_messages() {
  {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    messages_.clear();
  }
  tree_model_->clear();
  update_status();
}

void SyslogDialog::export_to_file(const std::string& filename) {
  std::lock_guard<std::mutex> lock(messages_mutex_);

  std::ofstream file(filename);
  if (!file) {
    return;
  }

  file
      << "Timestamp,Severity,Facility,Source IP,Hostname,Application,Message\n";

  for (const auto& msg : messages_) {
    file << msg.timestamp_string() << "," << msg.severity_string() << ","
         << msg.facility_string() << "," << msg.source_ip << "," << msg.hostname
         << "," << msg.application << "," << "\"" << msg.message << "\"\n";
  }
}

void SyslogDialog::on_start_clicked() { start_listening(); }

void SyslogDialog::on_stop_clicked() { stop_listening(); }

void SyslogDialog::on_clear_clicked() { clear_messages(); }

void SyslogDialog::on_export_clicked() {
  Gtk::FileChooserDialog dialog("Export to CSV", Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Save", Gtk::RESPONSE_OK);

  auto filter_csv = Gtk::FileFilter::create();
  filter_csv->set_name("CSV files");
  filter_csv->add_pattern("*.csv");
  dialog.add_filter(filter_csv);

  int result = dialog.run();

  if (result == Gtk::RESPONSE_OK) {
    std::string filename = dialog.get_filename();
    if (filename.find(".csv") == std::string::npos) {
      filename += ".csv";
    }
    export_to_file(filename);
  }
}

void SyslogDialog::on_filter_changed() {
  filtered_model_->refilter();
  save_config();
}

void SyslogDialog::on_message_received(const SyslogMessage& msg) {
  {
    std::lock_guard<std::mutex> lock(messages_mutex_);
    messages_.push_back(msg);
  }

  // Write to file if logging is enabled
  if (file_logging_enabled_) {
    write_message_to_file(msg);
  }

  {
    std::lock_guard<std::mutex> lock(pending_mutex_);
    pending_message_ = msg;
  }

  message_dispatcher_.emit();
}

void SyslogDialog::add_message_to_view(const SyslogMessage& msg) {
  SyslogMessage local_msg;
  {
    std::lock_guard<std::mutex> lock(pending_mutex_);
    local_msg = pending_message_;
  }

  auto row = *(tree_model_->append());
  row[columns_.timestamp] = local_msg.timestamp_string();
  row[columns_.severity] = local_msg.severity_string();
  row[columns_.facility] = local_msg.facility_string();
  row[columns_.source_ip] = local_msg.source_ip;
  row[columns_.hostname] = local_msg.hostname;
  row[columns_.application] = local_msg.application;
  row[columns_.message] = local_msg.message;
  row[columns_.severity_enum] = static_cast<int>(local_msg.severity);

  // Auto-scroll to new message
  auto adj = scrolled_window_.get_vadjustment();
  adj->set_value(adj->get_upper() - adj->get_page_size());

  update_status();
}

void SyslogDialog::on_message_dispatch() {
  // Wrapper for dispatcher - reads from pending_message_
  SyslogMessage local_msg;
  {
    std::lock_guard<std::mutex> lock(pending_mutex_);
    local_msg = pending_message_;
  }

  auto row = *(tree_model_->append());
  row[columns_.timestamp] = local_msg.timestamp_string();
  row[columns_.severity] = local_msg.severity_string();
  row[columns_.facility] = local_msg.facility_string();
  row[columns_.source_ip] = local_msg.source_ip;
  row[columns_.hostname] = local_msg.hostname;
  row[columns_.application] = local_msg.application;
  row[columns_.message] = local_msg.message;
  row[columns_.severity_enum] = static_cast<int>(local_msg.severity);

  // Auto-scroll to new message
  auto adj = scrolled_window_.get_vadjustment();
  adj->set_value(adj->get_upper() - adj->get_page_size());

  update_status();
}

bool SyslogDialog::filter_func(const Gtk::TreeModel::const_iterator& iter) {
  // Text filter
  std::string filter_text = filter_entry_.get_text();
  if (!filter_text.empty()) {
    Glib::ustring message = (*iter)[columns_.message];
    Glib::ustring hostname = (*iter)[columns_.hostname];
    Glib::ustring application = (*iter)[columns_.application];

    if (message.lowercase().find(filter_text) == Glib::ustring::npos &&
        hostname.lowercase().find(filter_text) == Glib::ustring::npos &&
        application.lowercase().find(filter_text) == Glib::ustring::npos) {
      return false;
    }
  }

  // Severity filter
  int severity = (*iter)[columns_.severity_enum];

  switch (severity) {
    case 0:
      return emergency_check_.get_active();
    case 1:
      return alert_check_.get_active();
    case 2:
      return critical_check_.get_active();
    case 3:
      return error_check_.get_active();
    case 4:
      return warning_check_.get_active();
    case 5:
      return notice_check_.get_active();
    case 6:
      return info_check_.get_active();
    case 7:
      return debug_check_.get_active();
    default:
      return true;
  }
}

void SyslogDialog::update_status() {
  std::lock_guard<std::mutex> lock(messages_mutex_);

  std::string status = "";
  if (listener_->is_listening()) {
    status = "Listening on port " + std::to_string(listener_->get_port());
  } else {
    status = "Not listening";
  }

  status += " (" + std::to_string(messages_.size()) + " messages)";

  if (file_logging_enabled_) {
    status += " [Logging to file]";
  }

  status_label_.set_text(status);
}

void SyslogDialog::enable_file_logging(bool enabled) {
  enable_logging_check_.set_active(enabled);
}

bool SyslogDialog::is_file_logging_enabled() const {
  return file_logging_enabled_;
}

void SyslogDialog::set_log_file_path(const std::string& path) {
  log_file_path_ = path;
  log_file_entry_.set_text(path);
}

std::string SyslogDialog::get_log_file_path() const { return log_file_path_; }

void SyslogDialog::on_logging_toggled() {
  file_logging_enabled_ = enable_logging_check_.get_active();

  if (file_logging_enabled_) {
    // Open log file in append mode
    std::lock_guard<std::mutex> lock(log_file_mutex_);
    log_file_path_ = log_file_entry_.get_text();

    log_file_stream_.open(log_file_path_, std::ios::out | std::ios::app);

    if (!log_file_stream_.is_open()) {
      Gtk::MessageDialog dialog("Error opening log file: " + log_file_path_,
                                false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK,
                                true);
      dialog.run();
      file_logging_enabled_ = false;
      enable_logging_check_.set_active(false);
    } else {
      // Write header if file is new/empty
      log_file_stream_.seekp(0, std::ios::end);
      if (log_file_stream_.tellp() == 0) {
        log_file_stream_ << "# Syslog Viewer Log File\n";
        log_file_stream_ << "# Format: Timestamp|Severity|Facility|Source "
                            "IP|Hostname|Application|Message\n";
      }
      log_file_stream_.flush();
    }
  } else {
    // Close log file
    std::lock_guard<std::mutex> lock(log_file_mutex_);
    if (log_file_stream_.is_open()) {
      log_file_stream_.close();
    }
  }

  update_status();
}

void SyslogDialog::on_log_file_browse_clicked() {
  Gtk::FileChooserDialog dialog("Select Log File",
                                Gtk::FILE_CHOOSER_ACTION_SAVE);
  dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
  dialog.add_button("Select", Gtk::RESPONSE_OK);

  auto filter_log = Gtk::FileFilter::create();
  filter_log->set_name("Log files");
  filter_log->add_pattern("*.log");
  dialog.add_filter(filter_log);

  auto filter_all = Gtk::FileFilter::create();
  filter_all->set_name("All files");
  filter_all->add_pattern("*");
  dialog.add_filter(filter_all);

  // Set current filename if exists
  if (!log_file_path_.empty()) {
    dialog.set_filename(log_file_path_);
  }

  int result = dialog.run();

  if (result == Gtk::RESPONSE_OK) {
    std::string filename = dialog.get_filename();
    if (filename.find(".log") == std::string::npos) {
      filename += ".log";
    }
    log_file_entry_.set_text(filename);
    log_file_path_ = filename;

    // If logging is already enabled, reopen with new file
    if (file_logging_enabled_) {
      enable_logging_check_.set_active(false);
      enable_logging_check_.set_active(true);
    }
  }
}

void SyslogDialog::write_message_to_file(const SyslogMessage& msg) {
  std::lock_guard<std::mutex> lock(log_file_mutex_);

  if (!log_file_stream_.is_open()) {
    return;
  }

  // Write message in pipe-delimited format
  log_file_stream_ << msg.timestamp_string() << "|" << msg.severity_string()
                   << "|" << msg.facility_string() << "|" << msg.source_ip
                   << "|" << msg.hostname << "|" << msg.application << "|"
                   << msg.message << "\n";

  log_file_stream_.flush();
}

std::string SyslogDialog::get_config_path() const {
  const char* home = getenv("HOME");
  if (!home) {
    home = getenv("USERPROFILE");  // Windows fallback
  }
  if (!home) {
    return "/tmp/syslog_viewer.conf";
  }
  return std::string(home) + "/.syslog_viewer.conf";
}

void SyslogDialog::load_config() {
  loading_config_ = true;

  std::string config_path = get_config_path();
  std::ifstream config_file(config_path);

  if (!config_file.is_open()) {
    loading_config_ = false;
    return;  // No config file, use defaults
  }

  std::string line;
  while (std::getline(config_file, line)) {
    // Skip empty lines and comments
    if (line.empty() || line[0] == '#') {
      continue;
    }

    // Parse key=value pairs
    size_t pos = line.find('=');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);

      if (key == "port") {
        try {
          int port = std::stoi(value);
          if (port > 0 && port <= 65535) {
            port_spin_.set_value(port);
            listener_->set_port(port);
          }
        } catch (...) {
          // Invalid port value, ignore
        }
      } else if (key == "filter_emergency") {
        emergency_check_.set_active(value == "1");
      } else if (key == "filter_alert") {
        alert_check_.set_active(value == "1");
      } else if (key == "filter_critical") {
        critical_check_.set_active(value == "1");
      } else if (key == "filter_error") {
        error_check_.set_active(value == "1");
      } else if (key == "filter_warning") {
        warning_check_.set_active(value == "1");
      } else if (key == "filter_notice") {
        notice_check_.set_active(value == "1");
      } else if (key == "filter_info") {
        info_check_.set_active(value == "1");
      } else if (key == "filter_debug") {
        debug_check_.set_active(value == "1");
      }
    }
  }

  config_file.close();
  loading_config_ = false;
}

void SyslogDialog::save_config() {
  // Don't save while loading config
  if (loading_config_) {
    return;
  }

  std::string config_path = get_config_path();
  std::ofstream config_file(config_path);

  if (!config_file.is_open()) {
    return;  // Can't save config
  }

  config_file << "# Syslog Viewer Configuration\n";
  config_file << "port=" << static_cast<int>(port_spin_.get_value()) << "\n";

  // Save filter checkbox states
  config_file << "filter_emergency=" << (emergency_check_.get_active() ? "1" : "0") << "\n";
  config_file << "filter_alert=" << (alert_check_.get_active() ? "1" : "0") << "\n";
  config_file << "filter_critical=" << (critical_check_.get_active() ? "1" : "0") << "\n";
  config_file << "filter_error=" << (error_check_.get_active() ? "1" : "0") << "\n";
  config_file << "filter_warning=" << (warning_check_.get_active() ? "1" : "0") << "\n";
  config_file << "filter_notice=" << (notice_check_.get_active() ? "1" : "0") << "\n";
  config_file << "filter_info=" << (info_check_.get_active() ? "1" : "0") << "\n";
  config_file << "filter_debug=" << (debug_check_.get_active() ? "1" : "0") << "\n";

  config_file.close();
}
