#include <dpp/dpp.h>
#include <fstream>
#include <iterator>
#include "include/scr.h"

using json = nlohmann::json;

/**
 *  1.  Store every message sent in the server.
 *  2.  Be able to retrieve every message sent via it's id
 *  3.  Globally create bulk commands
 *  4.  Command to get "real" size of cache
 *  5.  Ignore messages from users with a (vector?) of roles
 *  6.  Command to return count of message_cache
 *  7.  dpp::cache with std::map
 *  8.  Retrieve sniped message
 *  9.  Filter by channel id
 *  10. Make class for std::map (with list/dequeue) to track key insertion order
 *  11. Create RAM Checker to test memory footprint
 *  12. Makes more sense to use dpp::snowflake than uint64_t since dpp has good methods for using snowflake
 *  13. Edit sniping
 */

/**
 *  Storage Structure:
 *  dpp::cache -> std::map on message delete event (message is found through id)
 *  if message is not found in cache (i.e too old), don't store
 *  dpp::cache is automatically cleaned
 *  delete_cache is automatically cleaned
 *  cache_id is map.size
 */

/**
 *  git init
 *  connect to remote repo :3
 */

/**
 * scr namespace
 *
 * cache class
 * -    Ordered Map for cache
 */

int main()
{
    std::ifstream f("./src/token.json");
    if (!f.is_open())
        std::cerr << "Unable to open file";
    json data = json::parse(f);
    f.close();

    int intents = dpp::i_all_intents;
    dpp::cluster bot(data["token"], intents);

    bot.on_log(dpp::utility::cout_logger());

    scr::cache<dpp::message> message_cache;
    scr::cache<dpp::message> snipe_cache;
    scr::cache<dpp::message> edit_cache;

    bot.on_message_create([&bot, &message_cache](const dpp::message_create_t &e)
    {
        if (e.msg.author.is_bot()) return;
        dpp::message *m = new dpp::message();
        *m = e.msg;
        message_cache.store(m, false); 
    });

    bot.on_message_delete([&bot, &snipe_cache, &message_cache](const dpp::message_delete_t &e)
    {
        if (snipe_cache.count() > MAX_SNIPE_SIZE) { snipe_cache.clean_amount(REMOVE_SNIPE); };
        if (message_cache.count() > MAX_STORAGE_SIZE) {message_cache.clean_amount(REMOVE_STORAGE); };
        uint64_t id = e.id;
        dpp::message *msg = message_cache.find(id);
        if (msg == nullptr) return; 
        snipe_cache.store(msg, false); 
    });

    bot.on_message_update([&bot, &edit_cache](const dpp::message_update_t &e)
    {
        if (e.msg.author.is_bot()) return;
        dpp::message *m = new dpp::message();
        *m = e.msg;
        edit_cache.store(m, false); 
    });

    bot.on_slashcommand([&bot, &message_cache, &snipe_cache, &edit_cache](const dpp::slashcommand_t &e)
    {
        if (e.command.get_command_name() == "snipe")
        {
            if (snipe_cache.empty()) return;
            // uint64_t channel_id = e.command.channel_id; 
            // std::optional<dpp::message*> message_filtered = scr::filter_by_channel_id(snipe_cache, channel_id);
            // if (!message_filtered.has_value()) e.reply("There is nothing to snipe!"); return;
            // dpp::message *message = message_filtered.value();

            // std::vector<dpp::attachment> *attachments = new std::vector<dpp::attachment>; 
            // *attachments = message->attachments;

            //  uint64_t message_ref_id = message->message_reference.message_id;
            // if (message_ref_id != 0)
            // {
            //     dpp::embed r_reference_embed = scr::reply_embed(*message);
            //     dpp::message m(channel_id, r_reference_embed);
            //     e.reply(m);
            // }
            e.reply("????");

            // uint64_t id = e.command.channel_id;
            // if (!snipe_cache.empty())
            // {
            //     std::optional<dpp::message*> r = scr::filter_by_channel_id(snipe_cache, id);
            //     if (!r.has_value())
            //     {
            //         e.reply("There is nothing to snipe!");
            //         return;
            //     }
            //     dpp::message *m = r.value();
            //     uint64_t x = m->message_reference.message_id;
                
            //     // Reply
            //     if (x != 0 )
            //     {
            //         dpp::snowflake m_id = x;
            //         dpp::message *tm = message_cache.find(m_id);
            //         e.reply(tm->content);
            //     }
            //     e.reply(m->content);
            //     return;
            // }
            // e.reply("There is nothing to snipe!");
       }
       else if (e.command.get_command_name() == "esnipe")
       {
           uint64_t id = e.command.channel_id;
           if (!edit_cache.empty())
           {
               std::optional<dpp::message *> r = scr::filter_by_channel_id(edit_cache, id);
               if (!r.has_value())
               {
                   e.reply("There is nothing to snipe!");
                   return;
               }

               // Check if OG message was deleted


               dpp::message *m = r.value();
               e.reply(m->content);
               return;
           }
           e.reply("There is nothing to snipe!");
       } 
    });

    // COMMANDS (3)
    dpp::slashcommand snipe_command("snipe", "snipes a message or something idk", bot.me.id);

    dpp::slashcommand edit_snipe_command("esnipe", "snipes an edited message", bot.me.id);

    dpp::slashcommand cache_size("sizeofcache", "returns the size of cache", bot.me.id);

    dpp::slashcommand count("count", "returns the amount of messages stored", bot.me.id);

    const std::vector<dpp::slashcommand> commands = {snipe_command, cache_size, count, edit_snipe_command};

    bot.on_ready([&bot, &commands, &snipe_command](const dpp::ready_t &event)
    {
        if (dpp::run_once<struct register_bot_commands>()) 
        {
            bot.global_bulk_command_create(commands);
        } 
    });

    bot.start(dpp::st_wait);
}