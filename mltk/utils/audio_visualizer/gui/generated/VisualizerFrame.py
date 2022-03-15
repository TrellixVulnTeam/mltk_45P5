# -*- coding: UTF-8 -*-
#
# generated by wxGlade 1.0.1 on Wed Apr 28 17:07:33 2021
#

import wx
# begin wxGlade: dependencies
# end wxGlade

# begin wxGlade: extracode
from ..controls_panel import VisualizerControlsPanel
from .. import matplotlib_spec_canvas
from ..generated import add_widget
# end wxGlade


class VisualizerFrame(wx.Frame):
    def __init__(self, *args, **kwds):
        # begin wxGlade: VisualizerFrame.__init__
        kwds["style"] = kwds.get("style", 0) | wx.DEFAULT_FRAME_STYLE
        wx.Frame.__init__(self, *args, **kwds)
        self.SetSize((1600, 1000))
        self.SetTitle("AudioFeatureGenerator Visualizer")

        # Menu Bar
        self.frame_menubar = wx.MenuBar()
        wxglade_tmp_menu = wx.Menu()
        item = wxglade_tmp_menu.Append(wx.ID_ANY, "View Read Me", "")
        self.Bind(wx.EVT_MENU, self.on_display_readme, item)
        self.frame_menubar.Append(wxglade_tmp_menu, "Help")
        self.SetMenuBar(self.frame_menubar)
        # Menu Bar end

        sizer_1 = wx.BoxSizer(wx.VERTICAL)

        self.window_1 = wx.SplitterWindow(self, wx.ID_ANY, style=wx.SP_3D | wx.SP_BORDER)
        self.window_1.SetMinimumPaneSize(20)
        sizer_1.Add(self.window_1, 1, wx.EXPAND, 0)

        self.window_1_pane_1 = wx.Panel(self.window_1, wx.ID_ANY)

        sizer_2 = wx.BoxSizer(wx.VERTICAL)

        self.spectrogram_canvas = matplotlib_spec_canvas.MatplotlibSpectrogramCanvas(self.window_1_pane_1, wx.ID_ANY)
        add_widget('spectrogram_canvas', self.spectrogram_canvas)
        sizer_2.Add(self.spectrogram_canvas, 2, wx.ALL | wx.EXPAND, 3)

        self.window_24 = VisualizerControlsPanel(self.window_1, wx.ID_ANY)
        self.window_24.SetupScrolling()

        self.window_1_pane_1.SetSizer(sizer_2)

        self.window_1.SplitVertically(self.window_1_pane_1, self.window_24, 967)

        self.SetSizer(sizer_1)

        self.Layout()

        # end wxGlade

    def on_display_readme(self, event):  # wxGlade: VisualizerFrame.<event_handler>
        print("Event handler 'on_display_readme' not implemented!")
        event.Skip()

# end of class VisualizerFrame
