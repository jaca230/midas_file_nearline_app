# cmake/CPMConfig.cmake

# ---------------------- CPM Package Registry ----------------------
# NOTE: Order matters! Dependencies should be declared in dependency order.
# NOTE: The name in package list matters! Dependencies use "Find{name}.cmake",
# so the name has to match the normal package export name for that package
# NOTE: "CMAKE_POSITION_INDEPENDENT_CODE ON" doesn't actually do anything for most systems. But
# the plugins are all shared libraries, so we set it to be sure. It also helps show where to set options.



set(CPM_PACKAGE_LIST
  #midas_receiver
  TBB                                   # required by analysis pipeline
  analysis_pipeline_core                # required by all analysis_pipeline libraries/plugins
  analysis_pipeline                     # required by all libraries/plugins that make internal pipelines
  unpacker_data_products_core           # required by all unpackers libraries/plugins
  midas_event_unpacker_plugin           # required by midas event unpackers like sbfoster_miads_event_unpacker_plugin
  DataProducts                          # required by sbfoster unpackers and analyzers using unpacked data products
  Unpackers                             # required by sbfoster unpacking stage
  sbfoster_midas_event_unpacker_plugin  # not required by subsequent plugins
)

# ---------------------- midas_receiver ----------------------
set(midas_receiver_REPO   "jaca230/midas_receiver")
set(midas_receiver_TAG    "main")
set(midas_receiver_TARGET "MidasReceiver::midas_receiver")
set(midas_receiver_OPTIONS
  "CMAKE_POSITION_INDEPENDENT_CODE ON"
)


# ---------------------- oneTBB ----------------------
set(TBB_REPO   "oneapi-src/oneTBB")
set(TBB_TAG    "v2021.12.0")  # Or latest stable
set(TBB_TARGET "") # Public Dependecy of Analysis Pipeline, no need to link
set(TBB_OPTIONS
  "TBB_TEST OFF;TBB_STRICT OFF;TBB_EXAMPLES OFF;CMAKE_POSITION_INDEPENDENT_CODE ON"
)

# ---------------------- analysis_pipeline_core ----------------------
set(analysis_pipeline_core_REPO   "jaca230/analysis_pipeline_core")
set(analysis_pipeline_core_TAG    "main")
set(analysis_pipeline_core_TARGETS
  analysis_pipeline::analysis_pipeline_core
  analysis_pipeline::spdlog_header_only
  analysis_pipeline::nlohmann_json_header_only
)
set(analysis_pipeline_core_OPTIONS
  "CMAKE_POSITION_INDEPENDENT_CODE ON"
)


# ---------------------- analysis_pipeline ----------------------
set(analysis_pipeline_REPO   "jaca230/analysis_pipeline")
set(analysis_pipeline_TAG    "main")
set(analysis_pipeline_TARGET "analysis_pipeline::analysis_pipeline")
set(analysis_pipeline_OPTIONS
  "CMAKE_POSITION_INDEPENDENT_CODE ON"
  "BUILD_EXAMPLE_PLUGIN OFF"
)

# ---------------------- unpacker_data_products_core ----------------------
set(unpacker_data_products_core_REPO   "jaca230/unpacker_data_products_core")
set(unpacker_data_products_core_TAG    "main")
set(unpacker_data_products_core_TARGET "") # Dynamically linked
set(unpacker_data_products_coren_OPTIONS
  "CMAKE_POSITION_INDEPENDENT_CODE ON"
)


# ---------------------- midas_event_unpacker_plugin ----------------------
set(midas_event_unpacker_plugin_REPO   "jaca230/midas_event_unpacker_plugin")
set(midas_event_unpacker_plugin_TAG    "main")
set(midas_event_unpacker_plugin_TARGET "analysis_pipeline::midas_event_unpacker_plugin")
set(midas_event_unpacker_plugin_OPTIONS
  "CMAKE_POSITION_INDEPENDENT_CODE ON"
  "USE_BUNDLED_MIDAS ON"
)

# ---------------------- mu-data-products ----------------------
set(DataProducts_REPO   "sbfoster12/mu-data-products")
set(DataProducts_TAG    "main")
set(DataProducts_TARGET "DataProducts::data_products")
set(DataProducts_OPTIONS
  "CMAKE_POSITION_INDEPENDENT_CODE ON"
)

# ---------------------- mu-unpackers ----------------------
set(Unpackers_REPO   "sbfoster12/mu-unpackers")
set(Unpackers_TAG    "main")
set(Unpackers_TARGET "Unpackers::unpackers")
set(Unpackers_OPTIONS
  "CMAKE_POSITION_INDEPENDENT_CODE ON"
)


# ---------------------- custom_midas_unpacking_pipeline_plugin ----------------------
set(sbfoster_midas_event_unpacker_plugin_REPO   "jaca230/sbfoster_midas_event_unpacker_plugin")
set(sbfoster_midas_event_unpacker_plugin_TAG    "PSI_testbeam_august_2025")
set(sbfoster_midas_event_unpacker_plugin_TARGET "") # Dynamically linked
set(sbfoster_midas_event_unpacker_plugin_OPTIONS
  "CMAKE_POSITION_INDEPENDENT_CODE ON"
)
