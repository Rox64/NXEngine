DEB_NAME := CaveStory

WORK_DIR := deb
DEB_DIR := $(WORK_DIR)/$(DEB_NAME)
DEB_OUT := $(WORK_DIR)/$(DEB_NAME).deb
DEB_CONTROL := $(DEB_DIR)/DEBIAN/control
SITE_DIR := $(WORK_DIR)/site
REPO_DIR := $(SITE_DIR)/repo

PERL := /usr/bin/perl

INFO_PLIST := $(DEB_DIR)/Applications/CaveStory.app/Info.plist

ifeq (,$(wildcard $(INFO_PLIST)))
$(error No file $(INFO_PLIST). Maybe you forgot to execute "xcodebuild install")
endif

BUNDLE_ID   := $(shell /usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" $(INFO_PLIST))
BUNDLE_NAME := $(shell /usr/libexec/PlistBuddy -c "Print :CFBundleName" $(INFO_PLIST))

VERSION := $(shell git describe --tags | sed 's/\(.*\)-.*/\1/')

$(info $(VERSION) $(BUNDLE_NAME) $(BUNDLE_ID))

all: 



$(dir $(DEB_CONTROL)):
	mkdir -p $@

control: control.m4 $(dir $(DEB_CONTROL))
	m4 -DPACKAGE_ID="$(BUNDLE_ID)" -DPACKAGE_VERSION="$(VERSION)" control.m4 > $(DEB_CONTROL)

deb: control
	cd $(WORK_DIR); dpkg -b $(DEB_NAME)

$(SITE_DIR):
	mkdir -p $(SITE_DIR); cd $(SITE_DIR); git init; git remote add -t gh-pages -f origin git@github.com:PIlin/NXEngine-iOS.git; git checkout gh-pages	

$(REPO_DIR): $(SITE_DIR)
	mkdir -p $(REPO_DIR)

prepare_deploy: $(REPO_DIR)
	#cd $(SITE_DIR); git pull origin gh-pages

deploy: prepare_deploy
	cp -fv $(DEB_OUT) $(REPO_DIR)
	cp -v dpkg-scanpackages dpkg-gettext.pl $(REPO_DIR) 
	cd $(REPO_DIR); $(PERL) dpkg-scanpackages -m . /dev/null | bzip2 -zc > Packages.bz2
	cd $(REPO_DIR); git commit -a --amend -m "Package $(VERSION)"; git push --verbose --force origin gh-pages


.PHONY: all control deb prepare_deploy deploy
