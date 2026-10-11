from data.recipes import get_all_recipes

# 根据ID查食谱
def get_recipe_by_id(recipe_id):
    for recipe in get_all_recipes():
        if recipe["id"] == recipe_id:
            return recipe
    return None

# 随机选食谱
def get_random_recipes(count=3):
    import random
    all_rec = get_all_recipes()
    return random.sample(all_rec, min(count, len(all_rec)))

# 按食材/器械筛选
def filter_recipes(ingredients=[], equipment=[]):
    all_rec = get_all_recipes()
    return [r for r in all_rec
            if (not ingredients or any(i in r["ingredients"] for i in ingredients))
            and (not equipment or any(e in r["equipment"] for e in equipment))]