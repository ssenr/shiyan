#include<dpp/dpp.h>

/**
 *  AMOUNT TO CLEAN
 */

#define MAX_STORAGE_SIZE    3
#define MAX_SNIPE_SIZE      3 
#define REMOVE_STORAGE      2
#define REMOVE_SNIPE        3

/**
 *  scr::cache
 *  cache object using
 */
namespace scr
{
    template<class T>
    class cache
    {
        private:
            std::shared_mutex mtx;
            std::mutex d_mtx;
            std::unique_ptr<std::map<dpp::snowflake, T*>> map;
            std::unique_ptr<std::deque<dpp::snowflake>> order;
        public:
            cache()
            {
                map = std::make_unique<std::map<dpp::snowflake, T*>>();
                order = std::make_unique<std::deque<dpp::snowflake>>();
            }

            ~cache() = default;

            void print(dpp::cluster &bot)
            {
                std::shared_lock l(mtx);
                for (auto &it : map)
                {
                    bot.log(dpp::ll_info, it.second->content);
                }
            }

            void clean_amount(uint8_t x)
            {
                for (int i = 0; i < x && !order->empty(); ++i)
                {
                    dpp::snowflake first_key = order->front();
                    order->pop_front();
                    map->erase(first_key);
                }
            }

            uint64_t count()
            {
                std::shared_lock l(mtx);
                return map->size();
            }

            T* find(uint64_t id)
            {
                std::unique_lock l(mtx);
                auto f = map->find(id);
                if (f == map->end()) { return nullptr; }
                return f->second;
            } 

            void store(T* obj, bool replace)
            {
                if (!obj) { return; }

                std::unique_lock l(mtx);
                auto f = map->find(obj->id);
                if (f == map->end()) { (*map)[obj->id] = obj; }
                if (replace == true && obj == f->second)
                { 
                    std::lock_guard<std::mutex> d_lock(d_mtx);
                    map->erase(obj->id);
                    (*map)[obj->id] = obj;
                }

                // Add to Q
                order->push_back(obj->id);
            }

            bool empty() { return map->empty();}

            std::shared_mutex &get_mutex()  { return mtx; }

            std::map<dpp::snowflake, T*> &get_map() { return *map; }
   };

    std::optional<dpp::message*> filter_by_channel_id(scr::cache<dpp::message> &c, const dpp::snowflake &channel_id)
    {
        std::map<dpp::snowflake, dpp::message*> &tmap = c.get_map();
        std::shared_mutex &mtx = c.get_mutex();
        std::unique_lock l(mtx);

        for (auto it = tmap.rbegin(); it != tmap.rend(); ++it)
        {
            if (it->second->channel_id == channel_id) { return it->second; }
        }
        return std::nullopt; 
    }

    dpp::embed reply_embed(dpp::message &m)
    {
        dpp::embed embed = dpp::embed()
        .set_color(dpp::colors::gray_chateau)
        .set_image(m.author.get_avatar_url());

        return embed; 
    }
}