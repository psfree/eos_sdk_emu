using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace NemirtingasEmuLauncher.ViewModels
{
    public class ViewModelBase : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void RaiseAndSetIfChanged<TProp>(ref TProp property, TProp value, [CallerMemberName] string prop_name = "")
        {
            if (property == null || !property.Equals(value))
            {
                property = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(prop_name));
            }
        }
    }
}
