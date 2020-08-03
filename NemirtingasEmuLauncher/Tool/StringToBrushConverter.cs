using Avalonia.Data.Converters;
using Avalonia.Media.Imaging;
using System;
using System.Globalization;
using System.Reflection;

namespace NemirtingasEmuLauncher.Windows
{
    public class StringToBrushConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is string)
            {
                if (!string.IsNullOrWhiteSpace((string)value))
                {
                    return new Bitmap(value.ToString());
                }
                var current_assembly = Assembly.GetExecutingAssembly();
                return new Bitmap(current_assembly.GetManifestResourceStream("NemirtingasEmuLauncher.Resources.no_image.png"));
            }
            throw new ArgumentException("Value must be of type String");
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new Exception("No back convertion available");
        }
    }
}
