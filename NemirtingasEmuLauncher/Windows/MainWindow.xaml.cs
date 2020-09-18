using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System;
using System.ComponentModel;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.Serialization;
using Avalonia.Input;

namespace NemirtingasEmuLauncher.Windows
{
    public class MainWindow : Window
    {
        private ScrollViewer _scvAppList;
        public ScrollViewer ScvAppList => _scvAppList ??= this.FindControl<ScrollViewer>("scvAppList");

        public MainWindow()
        {
            InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
        }

        private void InitializeComponent()
        {
            AvaloniaXamlLoader.Load(this);
            LoadDataContext();
            this.Closing += OnWindowClosing;

            ScvAppList.AddHandler(DragDrop.DropEvent, ((ViewModels.MainWindowViewModel)DataContext).OnDropFiles);

            LoadWindowCfg();
        }

        public class SavedWindow
        {
            public int X { get; set; }
            public int Y { get; set; }
            public int Width { get; set; }
            public int Height { get; set; }
        };

        public void LoadDataContext()
        {
            var DataContext = new ViewModels.MainWindowViewModel(this);
            this.DataContext = null;
            this.DataContext = DataContext;
            ScvAppList.DataContext = DataContext.AppList;
        }

        private void LoadWindowCfg()
        {
            try
            {
                SavedWindow save = new SavedWindow();
                var xmlserializer = new XmlSerializer(save.GetType());

                using (FileStream file = File.Open(Path.Combine(EpicEmulator.LauncherFolder, "LauncherWindow.cfg"), FileMode.Open))
                {
                    save = (SavedWindow)xmlserializer.Deserialize(file);

                    this.Position = new PixelPoint(save.X, save.Y);
                    this.ClientSize = new Avalonia.Size(save.Width, save.Height);
                }
            }
            catch (Exception)
            {
            }
        }

        public void OnWindowClosing(object sender, CancelEventArgs e)
        {
            try
            {
                SavedWindow save = new SavedWindow();
                var xmlserializer = new XmlSerializer(save.GetType());
                var stringWriter = new StringWriter();

                XmlWriterSettings settings = new XmlWriterSettings();
                settings.Indent = true;
                settings.IndentChars = ("  ");
                settings.OmitXmlDeclaration = true;

                save.X = this.Position.X;
                save.Y = this.Position.Y;
                save.Width = (int)this.ClientSize.Width;
                save.Height = (int)this.ClientSize.Height;

                using (var writer = XmlWriter.Create(stringWriter, settings))
                {
                    xmlserializer.Serialize(writer, save);
                    using (FileStream file = File.Open(Path.Combine(EpicEmulator.LauncherFolder, "LauncherWindow.cfg"), FileMode.Create))
                    {
                        byte[] datas = new UTF8Encoding(false).GetBytes(stringWriter.ToString());
                        file.Write(datas, 0, datas.Length);
                    }
                }
            }
            catch (Exception)
            {
            }
        }
    }
}
