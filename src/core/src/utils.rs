/// Useful to sort by a reference to a String, which you can't do with sort_by_key due to
/// lifetimes.
pub fn sort_by_key_ref<T, F, K>(a: &mut [T], key: F)
where
    F: Fn(&T) -> &K,
    K: ?Sized + Ord,
{
    a.sort_by(|x, y| key(x).cmp(key(y)));
}

/// Useful to binary search by a reference to a String, which you can't do with
/// binary_search_by_key due to lifetimes.
pub fn binary_search_by_key_ref<T, F, K>(a: &[T], b: &K, key: F) -> Result<usize, usize>
where
    F: Fn(&T) -> &K,
    K: ?Sized + Ord,
{
    a.binary_search_by(|x| key(x).cmp(b))
}
