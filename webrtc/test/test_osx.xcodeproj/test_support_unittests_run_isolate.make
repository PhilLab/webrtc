all: \
    $(BUILT_PRODUCTS_DIR)/test_support_unittests.isolated

$(BUILT_PRODUCTS_DIR)/test_support_unittests.isolated \
    : \
    test_support_unittests.isolate \
    ../../tools/swarming_client/isolate.py \
    ../../tools/swarming_client/run_isolated.py
	@mkdir -p "$(BUILT_PRODUCTS_DIR)"
	python ../../tools/swarming_client/isolate.py check --result "$(BUILT_PRODUCTS_DIR)/test_support_unittests.isolated" --isolate "test_support_unittests.isolate" --path-variable DEPTH ../.. --path-variable PRODUCT_DIR "$(BUILT_PRODUCTS_DIR) " --config-variable "OS=mac" --config-variable "chromeos=0" --config-variable "component=static_library" --config-variable "internal_gles2_conform_tests=0" --config-variable "icu_use_data_file_flag=1" --config-variable "use_openssl=1" --extra-variable mac_product_name Chromium
