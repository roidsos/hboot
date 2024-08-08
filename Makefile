TARGET_CHECK:
	@if [ ! -f targets/$(TARGET).mk ]; then \
		echo "Error: TARGET file targets/$(TARGET).mk does not exist."; \
		exit 1; \
	fi

include targets/$(TARGET).mk

.PHONY: cleandist
cleandist:
	@rm -f $(KERNEL_CONFIG_PATH) .config