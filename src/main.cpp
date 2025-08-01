#include <filesystem>
#include <memory>
#include <string>
#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <spdlog/spdlog.h>

#include <TFile.h>
#include <TTree.h>

#include "midasio.h"
#include "analysis_pipeline/config/config_manager.h"
#include "analysis_pipeline/pipeline/pipeline.h"

int main(int argc, char** argv) {
    if (argc < 2 || argc > 4) {
        spdlog::error("Usage: {} input_midas_file [max_events] [--debug]", argv[0]);
        return EXIT_FAILURE;
    }

    const std::string input_file = argv[1];
    const int max_events_arg = (argc >= 3 && std::string(argv[2]).find("--") != 0)
                               ? std::stoi(argv[2])
                               : 10'000'000;

    if (max_events_arg <= 0) {
        spdlog::error("max_events must be a positive integer");
        return EXIT_FAILURE;
    }

    const size_t max_events = static_cast<size_t>(max_events_arg);
    const bool debug_mode = (argc >= 3 && std::string(argv[argc - 1]) == "--debug");

    spdlog::set_level(debug_mode ? spdlog::level::debug : spdlog::level::info);

    if (!std::filesystem::exists(input_file)) {
        spdlog::error("Input file does not exist: {}", input_file);
        return EXIT_FAILURE;
    }

    std::filesystem::path base_dir = std::filesystem::path(__FILE__).parent_path().parent_path();
    std::vector<std::string> config_files = {
        (base_dir / "config/logger.json").string(),
        (base_dir / "config/modes/default.json").string()
    };

    auto config_manager = std::make_shared<ConfigManager>();
    if (!config_manager->loadFiles(config_files) || !config_manager->validate()) {
        spdlog::error("Failed to load or validate config files");
        return EXIT_FAILURE;
    }

    Pipeline pipeline(config_manager);
    if (!pipeline.buildFromConfig()) {
        spdlog::error("Failed to build pipeline");
        return EXIT_FAILURE;
    }

    TMReaderInterface* count_reader = TMNewReader(input_file.c_str());
    if (!count_reader) {
        spdlog::error("Failed to open MIDAS file for counting: {}", input_file);
        return EXIT_FAILURE;
    }

    size_t total_events_in_file = 0;
    while (TMReadEvent(count_reader) != nullptr) {
        ++total_events_in_file;
    }
    delete count_reader;

    const size_t total_events_to_process = std::min(max_events, total_events_in_file);
    spdlog::debug("Total events in file: {}", total_events_in_file);
    spdlog::debug("Events to process: {}", total_events_to_process);

    const double progress_update_percent = 5.0;
    size_t next_progress_event = 0;
    size_t progress_step = static_cast<size_t>(total_events_to_process * (progress_update_percent / 100.0));
    if (progress_step == 0) progress_step = 1;

    TMReaderInterface* reader = TMNewReader(input_file.c_str());
    if (!reader) {
        spdlog::error("Failed to reopen MIDAS file: {}", input_file);
        return EXIT_FAILURE;
    }

    TFile output_file("output.root", "RECREATE");

    // Optional TTree output, currently disabled
    /*
    TTree tree("events", "Nalu unpacked events");
    dataProducts::NaluEvent* event_ptr = nullptr;
    dataProducts::NaluTime* time_ptr = nullptr;
    tree.Branch("event", &event_ptr);
    tree.Branch("times", &time_ptr);
    */

    size_t event_count = 0;
    auto t_start = std::chrono::steady_clock::now();

    while (event_count < total_events_to_process) {
        ++event_count;
        TMEvent* raw_event = TMReadEvent(reader);
        if (!raw_event) break;

        std::shared_ptr<TMEvent> wrapped_event(raw_event);
        InputBundle input;
        input.set("TMEvent", wrapped_event);
        pipeline.setInputData(std::move(input));
        pipeline.execute();

        auto& dpm = pipeline.getDataProductManager();

        // Serialize all data products to JSON string
        nlohmann::json serialized = dpm.serializeAll();

        // Print serialized JSON to stdout (one line per event)
        spdlog::info("{}", serialized.dump());

        dpm.clear();

        /*
        // Optional object extraction and filling
        auto event_lock = dpm.checkoutRead("NaluEvent");
        auto time_lock  = dpm.checkoutRead("NaluTime");
        event_ptr = dynamic_cast<dataProducts::NaluEvent*>(event_lock.get()->getObject());
        time_ptr  = dynamic_cast<dataProducts::NaluTime*>(time_lock.get()->getObject());
        if (event_ptr && time_ptr) {
            tree.Fill();
        }
        */

        if (event_count >= next_progress_event || event_count == total_events_to_process) {
            double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(
                std::chrono::steady_clock::now() - t_start
            ).count();

            double eps = (elapsed > 0) ? static_cast<double>(event_count) / elapsed : 0.0;
            double percent = 100.0 * event_count / total_events_to_process;
            double remaining_time = (eps > 0.0) ? (total_events_to_process - event_count) / eps : 0.0;

            spdlog::debug("[Progress] {:6.1f}% ({:7}/{}) | Time: {:7.2f} s | Rate: {:8.2f} events/s | ETA: {:7.2f} s",
                          percent, event_count, total_events_to_process, elapsed, eps, remaining_time);

            next_progress_event += progress_step;
        }
    }

    const auto t_end = std::chrono::steady_clock::now();
    const double duration_sec = std::chrono::duration_cast<std::chrono::duration<double>>(t_end - t_start).count();
    const double rate = (event_count > 0) ? static_cast<double>(event_count) / duration_sec : 0.0;

    output_file.cd();
    output_file.Write();
    output_file.Close();
    delete reader;

    spdlog::info("\n----------------------------------------");
    spdlog::info("           Processing Summary");
    spdlog::info("----------------------------------------");
    spdlog::info("{:<25} {:>10}", "Events processed:", event_count);
    spdlog::info("{:<25} {:>10.2f}", "Elapsed time (s):", duration_sec);
    spdlog::info("{:<25} {:>10.2f}", "Events per second:", rate);
    spdlog::info("{:<25} {}", "Output written to:", "output.root");
    spdlog::info("----------------------------------------");

    return EXIT_SUCCESS;
}
