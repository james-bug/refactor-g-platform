
include $(TOPDIR)/rules.mk

PKG_NAME:=gaming-platform
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/gaming-platform
  SECTION:=BenQ
  CATEGORY:=BenQ
  TITLE:=Gaming Platform Hardware Abstraction Layer
  SUBMENU:=Applications
  DEPENDS:=+libc
endef

define Package/gaming-platform/description
  Hardware Abstraction Layer for Gaming System.
  Provides unified interface for device detection, LED control,
  button input, and PS5 power management.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/. $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) \
		-fPIC -shared \
		-o $(PKG_BUILD_DIR)/libgaming-platform.so \
		$(PKG_BUILD_DIR)/platform_openwrt.c
endef

define Package/gaming-platform/install
	# 安裝共享庫
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libgaming-platform.so $(1)/usr/lib/

	# 安裝標頭檔（供其他套件使用）
	$(INSTALL_DIR) $(1)/usr/include/gaming
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/platform_interface.h $(1)/usr/include/gaming/
endef

$(eval $(call BuildPackage,gaming-platform))
