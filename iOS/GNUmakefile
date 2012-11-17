DEB_NAME := CaveStory

WORK_DIR := deb
DEB_DIR := $(WORK_DIR)/$(DEB_NAME)
#DEB_OUT := $(WORK_DIR)/$(DEB_NAME).deb
DEB_CONTROL := $(DEB_DIR)/DEBIAN/control



INFO_PLIST := $(DEB_DIR)/Applications/CaveStory.app/Info.plist

ifeq (,$(wildcard $(INFO_PLIST)))
$(error No file $(INFO_PLIST). Maybe you forgot to execute "xcodebuild install")
endif

BUNDLE_ID   := $(shell /usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" $(INFO_PLIST))
BUNDLE_NAME := $(shell /usr/libexec/PlistBuddy -c "Print :CFBundleName" $(INFO_PLIST))

VERSION := $(shell git describe --tags --long --dirty)

$(info $(VERSION) $(BUNDLE_NAME) $(BUNDLE_ID))

all: 



$(dir $(DEB_CONTROL)):
	mkdir -p $@

control: control.m4 $(dir $(DEB_CONTROL))
	m4 -DPACKAGE_ID="$(BUNDLE_ID)" -DPACKAGE_VERSION="$(VERSION)" control.m4 > $(DEB_CONTROL)

deb: control
	cd $(WORK_DIR); dpkg -b $(DEB_NAME)



.PHONY: all control deb