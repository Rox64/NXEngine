DEB_NAME := CaveStory

WORK_DIR := deb
DEB_DIR := $(WORK_DIR)/$(DEB_NAME)
DEB_OUT := $(WORK_DIR)/$(DEB_NAME).deb
DEB_CONTROL := $(DEB_DIR)/DEBIAN/control
SITE_DIR := $(WORK_DIR)/site
REPO_DIR := $(SITE_DIR)/repo
CHANGES_DIR := $(SITE_DIR)/changelogs

PERL := /usr/bin/perl

INFO_PLIST := $(DEB_DIR)/Applications/CaveStory.app/Info.plist

ifeq (,$(wildcard $(INFO_PLIST)))
$(error No file $(INFO_PLIST). Maybe you forgot to execute "xcodebuild install")
endif

BUNDLE_ID   := $(shell /usr/libexec/PlistBuddy -c "Print :CFBundleIdentifier" $(INFO_PLIST))
BUNDLE_NAME := $(shell /usr/libexec/PlistBuddy -c "Print :CFBundleName" $(INFO_PLIST))

VERSION := $(shell git describe --tags | sed 's/\(.*\)-.*/\1/')

$(info $(VERSION) $(BUNDLE_NAME) $(BUNDLE_ID))

# DEPICTION := http://10.0.0.97:8000/changelogs/$(VERSION).html
DEPICTION := http://pilin.github.io/NXEngine-iOS/changelogs/$(VERSION).html

DEPICTION_LAYOUT :=---\nlayout: cydia\n---

CHANGELOG_FILES := $(wildcard changelogs/*.md)
SITE_CHANGELOG_FILES := $(patsubst changelogs/%,$(CHANGES_DIR)/%,$(CHANGELOG_FILES))


all: 



$(dir $(DEB_CONTROL)):
	mkdir -p $@

control: control.m4 $(dir $(DEB_CONTROL))
	m4  -DPACKAGE_ID="$(BUNDLE_ID)" \
		-DPACKAGE_VERSION="$(VERSION)" \
		-DDEPICTION="$(DEPICTION)" \
		control.m4 > $(DEB_CONTROL)

deb: control
	cd $(WORK_DIR); dpkg -b $(DEB_NAME)

$(SITE_DIR):
	mkdir -p $(SITE_DIR); cd $(SITE_DIR); git init; git remote add -t gh-pages -f origin git@github.com:PIlin/NXEngine-iOS.git; git checkout gh-pages	

$(REPO_DIR): $(SITE_DIR)
	mkdir -p $(REPO_DIR)

$(CHANGES_DIR): 
	mkdir -p $@

$(CHANGES_DIR)/%.md: changelogs/%.md $(CHANGES_DIR)
	(echo "$(DEPICTION_LAYOUT)"; cat $<) > $@

changes: $(SITE_CHANGELOG_FILES)
	test -f "$(CHANGES_DIR)/$(VERSION).md" || ( echo 'Create changelog file' && false)

prepare_deploy: $(REPO_DIR) changes
	#cd $(SITE_DIR); git pull origin gh-pages
	cp -fv $(DEB_OUT) $(REPO_DIR)
	cp -v dpkg-scanpackages dpkg-gettext.pl $(REPO_DIR) 
	cd $(REPO_DIR); $(PERL) dpkg-scanpackages -m . /dev/null | bzip2 -zc > Packages.bz2

deploy: prepare_deploy
	cd $(SITE_DIR); \
		git add "changelogs/$(VERSION).md"
	cd $(REPO_DIR); \
		git commit -a --amend -m "Package $(VERSION)"; \
		git push --verbose --force origin gh-pages


.PHONY: all control deb prepare_deploy deploy changes
