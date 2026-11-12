# QVariant

    [cling]$ auto v = QVariant("123")
    (QVariant &) @0x111ab4568
    [cling]$ v.canConvert<int>()
    (bool) true
    [cling]$ v.canConvert<double>()
    (bool) true
    [cling]$ v.canConvert<float>()
    (bool) true
    [cling]$ v.canConvert<short>()
    (bool) true
    [cling]$ v.canConvert<QByteArray>()
    (bool) true
    [cling]$
