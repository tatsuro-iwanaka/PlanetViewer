GCC = g++ --std=c++2a -O3

MACOS_MIN = 14.0
TARGET = PlanetViewer
HOMEBREW_PREFIX = /opt/homebrew
CSPICE_DIR = $(HOMEBREW_PREFIX)/Cellar/cspice/67

CFLAGS = -I$(CSPICE_DIR)/include -I. -I./imgui -I./imgui/backends `pkg-config --cflags glfw3` -DGL_SILENCE_DEPRECATION -mmacosx-version-min=$(MACOS_MIN)
LIBS = $(CSPICE_DIR)/lib/libcspice.a \
       $(HOMEBREW_PREFIX)/lib/libglfw3.a \
       -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

IMGUI_SRCS = imgui/imgui.cpp imgui/imgui_draw.cpp imgui/imgui_widgets.cpp \
             imgui/imgui_tables.cpp imgui/backends/imgui_impl_glfw.cpp \
             imgui/backends/imgui_impl_opengl3.cpp

SRCS = main.cpp $(IMGUI_SRCS)

APP_NAME = PlanetViewer
APP_DIR = $(APP_NAME).app
CONTENTS = $(APP_DIR)/Contents
MACOS = $(CONTENTS)/MacOS
RESOURCES = $(CONTENTS)/Resources

all: $(TARGET)

$(TARGET): $(SRCS)
	$(GCC) $(CFLAGS) $(SRCS) $(LIBS) -mmacosx-version-min=$(MACOS_MIN) -o $(TARGET)

bundle: $(TARGET)
	@mkdir -p $(MACOS) $(RESOURCES)
	@cp $(TARGET) $(MACOS)/
	
	@vtool -set-build-version macos 11.0 15.0 -output $(MACOS)/$(TARGET).tmp $(MACOS)/$(TARGET)
	@mv $(MACOS)/$(TARGET).tmp $(MACOS)/$(TARGET)
	@chmod +x $(MACOS)/$(TARGET)
	
	@if [ -f "icon.icns" ]; then \
		cp icon.icns $(RESOURCES)/; \
		echo "Icon copied."; \
	else \
		echo "Warning: icon.icns not found."; \
	fi
	
	@if [ -d "kernels" ]; then cp -R kernels $(RESOURCES)/; fi
	
	@printf '<?xml version="1.0" encoding="UTF-8"?>\n\
	<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n\
	<plist version="1.0">\n\
	<dict>\n\
	<key>CFBundleExecutable</key><string>$(APP_NAME)</string>\n\
	<key>CFBundleIconFile</key><string>icon.icns</string>\n\
	<key>CFBundleIdentifier</key><string>tatsuro_iwanaka</string>\n\
	<key>CFBundleName</key><string>$(APP_NAME)</string>\n\
	<key>CFBundlePackageType</key><string>APPL</string>\n\
	<key>CFBundleShortVersionString</key><string>1.0</string>\n\
	<key>LSMinimumSystemVersion</key><string>11.0</string>\n\
	</dict>\n\
	</plist>' > $(CONTENTS)/Info.plist
	
	@codesign --force --deep --sign - $(APP_DIR)
	@echo "Bundle created and signed: $(APP_DIR)"

clean:
	rm -rf $(TARGET) $(APP_DIR)