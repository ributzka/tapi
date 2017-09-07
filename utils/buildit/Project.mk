BUILD_TAPI = $(shell find $(SRCROOT) -name build_tapi)

SUPPORTS_TEXT_BASED_API ?= YES
$(info SUPPORTS_TEXT_BASED_API=$(SUPPORTS_TEXT_BASED_API))

TAPI_INSTALL_PREFIX := $(DSTROOT)/$(TOOLCHAIN_INSTALL_DIR)
TAPI_LIBRARY_PATH := $(TAPI_INSTALL_PREFIX)/usr/lib

TAPI_COMMON_OPTS := -dynamiclib \
		    -xc++ \
		    -std=c++11 \
		    $(RC_ARCHS:%=-arch %) \
		    -install_name @rpath/libtapi.dylib \
		    -current_version $(RC_ProjectSourceVersion) \
		    -compatibility_version 1 \
		    -allowable_client ld \
		    -I$(DSTROOT)/$(TOOLCHAIN_INSTALL_DIR)/usr/local/include \
		    -o $(OBJROOT)/libtapi.tbd

TAPI_VERIFY_OPTS := $(TAPI_COMMON_OPTS) \
		    --verify-mode=Pedantic \
                    --verify-against=$(TAPI_LIBRARY_PATH)/libtapi.dylib

.PHONY: tapi installapi install-tapi installhdrs-tapi installapi-tapi tapi-build tapi-verify

ifeq ($(RC_ProjectName),tapi)
$(info TAPI Makefile)
installhdrs: installhdrs-tapi
installapi: installapi-tapi
endif

installhdrs-tapi: $(DSTROOT)
	@echo
	@echo ++++++++++++++++++++++
	@echo + Installing headers +
	@echo ++++++++++++++++++++++
	@echo
	ditto $(SRCROOT)/tools/clang/tools/tapi/include/tapi/*.h $(TAPI_INSTALL_PREFIX)/usr/local/include/tapi/
	# Generate Version.inc
	echo "$(RC_ProjectSourceVersion)" | awk -F. '{             \
	  printf "#define TAPI_VERSION %d.%d.%d\n", $$1, $$2, $$3; \
	  printf "#define TAPI_VERSION_MAJOR %dU\n", $$1;          \
	  printf "#define TAPI_VERSION_MINOR %dU\n", $$2;          \
	  printf "#define TAPI_VERSION_PATCH %dU\n", $$3;          \
	}' > $(TAPI_INSTALL_PREFIX)/usr/local/include/tapi/Version.inc

installapi-tapi: installhdrs-tapi $(OBJROOT) $(DSTROOT)
	@echo
	@echo ++++++++++++++++++++++
	@echo + Running InstallAPI +
	@echo ++++++++++++++++++++++
	@echo
	
	@if [ "$(SUPPORTS_TEXT_BASED_API)" != "YES" ]; then \
	  echo "installapi for target 'tapi' was requested, but SUPPORTS_TEXT_BASED_API has been disabled."; \
	  exit 1; \
	fi
	
	xcrun --sdk $(SDKROOT) tapi installapi \
	  $(TAPI_COMMON_OPTS) \
	  $(TAPI_INSTALL_PREFIX)
	
	$(INSTALL) -d -m 0755 $(TAPI_LIBRARY_PATH)
	$(INSTALL) -c -m 0755 $(OBJROOT)/libtapi.tbd $(TAPI_LIBRARY_PATH)/libtapi.tbd

install-tapi: tapi
tapi: tapi-build
ifeq ($(SUPPORTS_TEXT_BASED_API),YES)
tapi: tapi-verify
endif
tapi-build: $(OBJROOT) $(SYMROOT) $(DSTROOT)
	@echo
	@echo +++++++++++++++++++++
	@echo + Build and Install +
	@echo +++++++++++++++++++++
	@echo
	
	cd $(OBJROOT) && \
	  $(BUILD_TAPI) $(ENABLE_ASSERTIONS) $(LLVM_OPTIMIZED)

tapi-verify: tapi-build
	@echo
	@echo +++++++++++++++++++++++++++++++++
	@echo + Running InstallAPI and Verify +
	@echo +++++++++++++++++++++++++++++++++
	@echo
	
	@if [ "$(SUPPORTS_TEXT_BASED_API)" != "YES" ]; then \
	  echo "installapi for target 'tapi' was requested, but SUPPORTS_TEXT_BASED_API has been disabled."; \
	  exit 1; \
	fi
	
	xcrun --sdk $(SDKROOT) tapi installapi \
	  $(TAPI_VERIFY_OPTS) \
	  $(TAPI_INSTALL_PREFIX)
	  
	$(INSTALL) -d -m 0755 $(TAPI_LIBRARY_PATH)
	$(INSTALL) -c -m 0755 $(OBJROOT)/libtapi.tbd $(TAPI_LIBRARY_PATH)/libtapi.tbd

