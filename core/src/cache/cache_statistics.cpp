#include <cache/cache_statistics.hpp>

#include <formats/json/value_builder.hpp>

namespace cache {

UpdateStatistics CombineStatistics(const UpdateStatistics& a,
                                   const UpdateStatistics& b) {
  UpdateStatistics result;
  result.update_attempt_count = a.update_attempt_count + b.update_attempt_count;
  result.update_no_changes_count =
      a.update_no_changes_count + b.update_no_changes_count;
  result.update_failures_count =
      a.update_failures_count + b.update_failures_count;
  result.documents_read_count = a.documents_read_count + b.documents_read_count;
  result.documents_parse_failures =
      a.documents_parse_failures + b.documents_parse_failures;

  result.last_update_start_time = std::max(a.last_update_start_time.load(),
                                           b.last_update_start_time.load());
  result.last_successful_update_start_time =
      std::max(a.last_successful_update_start_time.load(),
               b.last_successful_update_start_time.load());
  result.last_update_duration =
      std::max(a.last_update_duration.load(), b.last_update_duration.load());

  return result;
}

namespace {

template <typename Clock, typename Duration>
std::int64_t TimeStampToMillisecondsFromNow(
    std::chrono::time_point<Clock, Duration> time) {
  const auto diff = Clock::now() - time;
  return std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
}

}  // namespace

formats::json::Value StatisticsToJson(const UpdateStatistics& stats) {
  formats::json::ValueBuilder result(formats::json::Type::kObject);

  formats::json::ValueBuilder update(formats::json::Type::kObject);
  update["attempts_count"] = stats.update_attempt_count.load();
  update["no_changes_count"] = stats.update_no_changes_count.load();
  update["failures_count"] = stats.update_failures_count.load();
  result["update"] = update.ExtractValue();

  formats::json::ValueBuilder documents(formats::json::Type::kObject);
  documents["read_count"] = stats.documents_read_count.load();
  documents["parse_failures"] = stats.documents_parse_failures.load();
  result["documents"] = documents.ExtractValue();

  formats::json::ValueBuilder age(formats::json::Type::kObject);
  age["time-from-last-update-start-ms"] =
      TimeStampToMillisecondsFromNow(stats.last_update_start_time.load());
  age["time-from-last-successful-start-ms"] = TimeStampToMillisecondsFromNow(
      stats.last_successful_update_start_time.load());
  age["last-update-duration-ms"] =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          stats.last_update_duration.load())
          .count();
  result["time"] = age.ExtractValue();

  return result.ExtractValue();
}

formats::json::Value StatisticsToJson(const DumpStatistics& stats) {
  formats::json::ValueBuilder result(formats::json::Type::kObject);

  const bool is_loaded = stats.is_loaded.load();
  result["is-loaded-from-dump"] = is_loaded ? 1 : 0;
  if (is_loaded) {
    result["load-duration-ms"] = stats.load_duration.load().count();
  }
  result["is-current-from-dump"] = stats.is_current_from_dump.load() ? 1 : 0;

  const bool dump_written = stats.last_nontrivial_write_start_time.load() !=
                            std::chrono::steady_clock::time_point{};
  if (dump_written) {
    formats::json::ValueBuilder write(formats::json::Type::kObject);
    write["time-from-start-ms"] = TimeStampToMillisecondsFromNow(
        stats.last_nontrivial_write_start_time.load());
    write["duration-ms"] = stats.last_nontrivial_write_duration.load().count();
    write["size-kb"] = stats.last_written_size.load() / 1024;
    result["last-nontrivial-write"] = write.ExtractValue();
  }

  return result.ExtractValue();
}

UpdateStatisticsScope::UpdateStatisticsScope(Statistics& stats,
                                             cache::UpdateType type)
    : stats_(stats),
      update_stats_(type == cache::UpdateType::kIncremental
                        ? stats.incremental_update
                        : stats.full_update),
      finished_(false),
      update_start_time_(std::chrono::steady_clock::now()) {
  update_stats_.last_update_start_time = update_start_time_;
  ++update_stats_.update_attempt_count;
}

UpdateStatisticsScope::~UpdateStatisticsScope() {
  if (!finished_) ++update_stats_.update_failures_count;
}

void UpdateStatisticsScope::Finish(size_t documents_count) {
  const auto update_stop_time = std::chrono::steady_clock::now();
  update_stats_.last_successful_update_start_time = update_start_time_;
  update_stats_.last_update_duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(update_stop_time -
                                                            update_start_time_);
  stats_.documents_current_count = documents_count;

  finished_ = true;
}

void UpdateStatisticsScope::FinishNoChanges() {
  ++update_stats_.update_no_changes_count;
  Finish(stats_.documents_current_count.load());
}

void UpdateStatisticsScope::IncreaseDocumentsReadCount(size_t add) {
  update_stats_.documents_read_count += add;
}

void UpdateStatisticsScope::IncreaseDocumentsParseFailures(size_t add) {
  update_stats_.documents_parse_failures += add;
}

}  // namespace cache
