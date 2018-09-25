from collections import defaultdict


class ListUtils:
    @classmethod
    def groupby2(cls, l, key, valueFn):
        new_dict = defaultdict(list)
        for value in l:
            new_dict[key(value)].append(valueFn(value))
        return new_dict

    @classmethod
    def groupby(cls, l, key):
        new_dict = defaultdict(list)
        for value in l:
            new_dict[key(value)].append(value)
        return new_dict

    @classmethod
    def sortbymultiple(cls, l, keys):
        copy = list(l)
        for k in reversed(keys):
            copy.sort(key=k)
        return copy

