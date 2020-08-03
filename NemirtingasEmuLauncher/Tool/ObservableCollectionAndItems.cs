using System.Collections.Generic;
using System.ComponentModel;
using System.Collections.Specialized;
using System.Collections;

namespace NemirtingasEmuLauncher.ViewModels
{
    public class ObservableCollectionAndItems<T> : INotifyCollectionChanged, IList<T>
        where T : INotifyPropertyChanged
    {

        #region IList<T>
        private List<T> _innerList = new List<T>();
        public int IndexOf(T item)
        {
            return _innerList.IndexOf(item);
        }

        public void Insert(int index, T item)
        {
            item.PropertyChanged += Item_PropertyChanged;
            _innerList.Insert(index, item);
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, item, index));
        }

        private void Item_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Replace, sender, sender, _innerList.IndexOf((T)sender)));
        }

        public void RemoveAt(int index)
        {
            _innerList[index].PropertyChanged -= Item_PropertyChanged;
            var item = _innerList[index];
            _innerList.RemoveAt(index);
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, item, index));
        }

        public T this[int index]
        {
            get => _innerList[index];
            set
            {
                var old = _innerList[index];
                _innerList[index] = value;
                RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Replace, new[] { value }, new[] { old }, index));
            }
        }

        public void Add(T item)
        {
            item.PropertyChanged += Item_PropertyChanged;
            _innerList.Add(item);
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, item, _innerList.Count-1));
        }


        public void Clear()
        {
            foreach (var item in _innerList)
            {
                item.PropertyChanged -= Item_PropertyChanged;
            }
            _innerList.Clear();
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
        }

        public bool Contains(T item)
        {
            return _innerList.Contains(item);
        }

        public void CopyTo(T[] array, int arrayIndex)
        {
            _innerList.CopyTo(array, arrayIndex);
        }

        public bool Remove(T item)
        {
            item.PropertyChanged -= Item_PropertyChanged;
            var index = _innerList.IndexOf(item);
            var ret = _innerList.Remove(item);
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, item, index));
            return ret;
        }

        public int Count => _innerList.Count;

        public bool IsReadOnly => false;

        public IEnumerator<T> GetEnumerator()
        {
            return _innerList.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return _innerList.GetEnumerator();
        }
        #endregion

        #region INotifyCollectionChanged
        public event NotifyCollectionChangedEventHandler CollectionChanged;
        private void RaiseCollectionChanged(NotifyCollectionChangedEventArgs e)
        {
            CollectionChanged?.Invoke(this, e);
        }

        #endregion
    }
}
