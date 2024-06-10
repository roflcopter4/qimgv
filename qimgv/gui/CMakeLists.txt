## SOURCES FOR QIMGV GUI

target_sources(qimgv PRIVATE
    CentralWidget.cpp
    ContextMenu.cpp
    FlowLayout.cpp
    IDirectoryView.cpp
    MainWindow.cpp

    customWidgets/ActionButton.cpp
    customWidgets/ClickableLabel.cpp
    customWidgets/ContextMenuItem.cpp
    customWidgets/FloatingWidget.cpp
    customWidgets/FloatingWidgetContainer.cpp
    customWidgets/IconButton.cpp
    customWidgets/IconWidget.cpp
    customWidgets/KeySequenceEdit.cpp
    customWidgets/MenuItem.cpp
    customWidgets/OverlayWidget.cpp
    customWidgets/PathSelectorMenuItem.cpp
    customWidgets/SidePanelWidget.cpp
    customWidgets/SlidePanel.cpp
    customWidgets/SpinBoxInputFix.cpp
    customWidgets/ThumbnailView.cpp
    customWidgets/ThumbnailWidget.cpp
    customWidgets/VideoSlider.cpp
    customWidgets/PushButtonFocusInd.cpp
    customWidgets/StyledComboBox.cpp
    customWidgets/ColorSelectorButton.cpp
    customWidgets/EntryInfoItem.cpp
    customWidgets/SsideBar.cpp

    dialogs/FileReplaceDialog.cpp
    dialogs/ResizeDialog.cpp
    dialogs/ScriptEditorDialog.cpp
    dialogs/SettingsDialog.cpp
    dialogs/ShortcutCreatorDialog.cpp
    dialogs/PrintDialog.cpp

    overlays/ChangeLogWindow.cpp
    overlays/ControlsOverlay.cpp
    overlays/CopyOverlay.cpp
    overlays/CropOverlay.cpp
    overlays/FloatingMessage.cpp
    overlays/FloatingMessageProxy.cpp
    overlays/FullscreenInfoOverlay.cpp
    overlays/FullscreenInfoOverlayProxy.cpp
    overlays/ImageInfoOverlay.cpp
    overlays/ImageInfoOverlayProxy.cpp
    overlays/MapOverlay.cpp
    overlays/RenameOverlay.cpp
    overlays/SaveConfirmOverlay.cpp
    overlays/VideoControls.cpp
    overlays/VideoControlsProxy.cpp
    overlays/ZoomIndicatorOverlay.cpp
    overlays/ZoomIndicatorOverlayProxy.cpp

    panels/cropPanel/CropPanel.cpp
    panels/mainPanel/MainPanel.cpp
    panels/mainPanel/ThumbnailStrip.cpp
    panels/mainPanel/ThumbnailStripProxy.cpp
    panels/sidePanel/SidePanel.cpp

    panels/infoBar/InfoBar.cpp
    panels/infoBar/InfoBarProxy.cpp

    viewers/DocumentWidget.cpp
    viewers/ImageViewerV2.cpp
    viewers/VideoPlayer.cpp
    viewers/VideoPlayerInitProxy.cpp
    viewers/ViewerWidget.cpp

    folderView/FolderGridView.cpp
    folderView/FolderView.cpp
    folderView/FolderViewProxy.cpp
    folderView/FileSystemModelCustom.cpp
    folderView/TreeViewCustom.cpp
    folderView/BookmarksWidget.cpp
    folderView/BookmarksItem.cpp
    folderView/FVOptionsPopup.cpp
)

# UI Files

target_sources(qimgv PRIVATE
    ContextMenu.ui

    dialogs/FileReplaceDialog.ui
    dialogs/ResizeDialog.ui
    dialogs/ScriptEditorDialog.ui
    dialogs/SettingsDialog.ui
    dialogs/ShortcutCreatorDialog.ui
    dialogs/PrintDialog.ui

    overlays/ChangeLogWindow.ui
    overlays/CopyOverlay.ui
    overlays/FloatingMessage.ui
    overlays/FullscreenInfoOverlay.ui
    overlays/ImageInfoOverlay.ui
    overlays/RenameOverlay.ui
    overlays/SaveConfirmOverlay.ui
    overlays/VideoControls.ui

    panels/cropPanel/CropPanel.ui
    panels/sidePanel/SidePanel.ui

    panels/infoBar/InfoBar.ui

    folderView/FolderView.ui
    folderView/FVOptionsPopup.ui
)
