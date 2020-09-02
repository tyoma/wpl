#include <wpl/controls/header.h>

#include "helpers.h"
#include "helpers-visual.h"
#include "Mockups.h"
#include "MockupsListView.h"
#include "predicates.h"

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			typedef columns_model::column column_t;
			enum drawing_event_type { item_background = 1, item_self = 4, };

			class tracking_header : public controls::header
			{
			public:
				struct drawing_event
				{
					template <typename T>
					drawing_event(drawing_event_type type_, gcontext &context_,
							const gcontext::rasterizer_ptr &rasterizer_, const agge::rect<T> &box_, index_type item_,
							unsigned state_, wstring text_ = L"")
						: type(type_), context(&context_), rasterizer(rasterizer_.get()), item(item_), state(state_),
							text(text_)
					{	box.x1 = box_.x1, box.y1 = box_.y1, box.x2 = box_.x2, box.y2 = box_.y2;	}

					drawing_event_type type;
					gcontext *context;
					gcontext::rasterizer_type *rasterizer;
					agge::rect<double> box;
					index_type item;
					unsigned state;
					wstring text;
				};

				using controls::header::item_state_flags;

			public:
				tracking_header()
					: item_height(0), reported_events(item_background | item_self)
				{	}

			public:
				mutable vector<drawing_event> events;
				double item_height;
				unsigned reported_events;

			private:
				virtual agge::real_t get_item_height() const
				{	return (agge::real_t)item_height;	}

				virtual void draw_item_background(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
					const agge::rect_r &box, index_type item, unsigned state) const
				{
					if (item_background & reported_events)
						events.push_back(drawing_event(item_background, ctx, rasterizer, box, item, state));
				}

				virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer,
					const agge::rect_r &box, index_type item, unsigned state, const wstring &text) const
				{
					if (item_self & reported_events)
						events.push_back(drawing_event(item_self, ctx, rasterizer, box, item, state, text));
				}
			};

			typedef tracking_header::item_state_flags item_state;

			struct rect_eq : eq
			{
				using eq::operator ();

				bool operator ()(const tracking_header::drawing_event &lhs,
					const tracking_header::drawing_event &rhs) const
				{
					return lhs.type == rhs.type && lhs.context == rhs.context && lhs.rasterizer == rhs.rasterizer
						&& (*this)(lhs.box, rhs.box) && lhs.item == rhs.item && lhs.state == rhs.state
						&& lhs.text == rhs.text;
				}

				template <typename T1, typename T2>
				bool operator ()(const agge::rect<T1> &lhs, const agge::rect<T2> &rhs) const
				{
					return (*this)(lhs.x1, rhs.x1) && (*this)(lhs.y1, rhs.y1)
						&& (*this)(lhs.x2, rhs.x2) && (*this)(lhs.y2, rhs.y2);
				}
			};
		}

		begin_test_suite( HeaderTests )
			mocks::font_loader fake_loader;
			shared_ptr<gcontext::surface_type> surface;
			shared_ptr<gcontext::renderer_type> ren;
			shared_ptr<gcontext::text_engine_type> text_engine;
			shared_ptr<gcontext> ctx;
			gcontext::rasterizer_ptr ras;
			view::positioned_native_views nviews;

			init( Init )
			{
				surface.reset(new gcontext::surface_type(1000, 1000, 0));
				ren.reset(new gcontext::renderer_type(1));
				text_engine.reset(new gcontext::text_engine_type(fake_loader, 0));
				ctx.reset(new gcontext(*surface, *ren, *text_engine, agge::zero()));
				ras.reset(new gcontext::rasterizer_type);
			}


			test( ConstructionOfListViewDoesNotDrawAnything )
			{
				// INIT / ACT
				tracking_header hdr;

				// ASSERT
				assert_is_empty(hdr.events);
			}


			test( NothingIsDrawnIfModelsAreMissing )
			{
				// INIT
				tracking_header hdr;

				hdr.resize(1000, 30, nviews);

				// ACT
				hdr.draw(*ctx, ras);

				// ASSERT
				assert_is_empty(hdr.events);
			}


			test( MouseEventsDoNothingWhenModelIsNotSet )
			{
				// INIT
				tracking_header hdr;

				hdr.resize(1000, 30, nviews);

				// ACT
				hdr.mouse_down(mouse_input::left, 0, 0, 0);
				hdr.mouse_move(mouse_input::left, 0, 0);
				hdr.mouse_up(mouse_input::left, 0, 0, 0);
			}


			test( ViewInvalidationIsRequestedOnModelInvalidation )
			{
				// INIT
				int invalidations = 0;
				tracking_header hdr;
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(L"x", 1);
				slot_connection conn = hdr.invalidate += [&] (const agge::rect_i *r) {
					++invalidations;
					assert_null(r);
				};

				hdr.set_model(m);
				hdr.resize(1000, 30, nviews);

				// ACT
				m->invalidated();

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				m->invalidated();
				m->invalidated();

				// ASSERT
				assert_equal(3, invalidations);

				// ACT
				hdr.set_model(nullptr);
				m->invalidated();

				// ASSERT
				assert_equal(3, invalidations);
			}


			test( DrawingSequenceIsItemBgThenFg )
			{
				// INIT
				tracking_header hdr;

				hdr.resize(400, 50, nviews);
				hdr.set_model(mocks::columns_model::create(L"abc", 123));

				// ACT
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference[] = {
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(0, 0, 123, 50), 0, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 123, 50), 0, 0, L"abc"),
				};

				assert_equal_pred(reference, hdr.events, rect_eq());
			}


			test( ColumnsAreDrawnInOrder )
			{
				// INIT
				tracking_header hdr;
				column_t c1[] = { column_t(L"abc", 10), column_t(L"abc zyx", 17), column_t(L"AA bbb Z", 25), };

				hdr.resize(1000, 33, nviews);
				hdr.set_model(mocks::columns_model::create(c1, columns_model::npos(), true));

				// ACT
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference1[] = {
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(0, 0, 10, 33), 0, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 10, 33), 0, 0, L"abc"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(10, 0, 27, 33), 1, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(10, 0, 27, 33), 1, 0, L"abc zyx"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(27, 0, 52, 33), 2, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(27, 0, 52, 33), 2, 0, L"AA bbb Z"),
				};

				assert_equal_pred(reference1, hdr.events, rect_eq());

				// INIT
				column_t c2[] = { column_t(L"lorem", 100), column_t(L"ipsum", 17), };

				hdr.resize(1000, 13, nviews);
				hdr.set_model(mocks::columns_model::create(c2, columns_model::npos(), true));
				hdr.events.clear();

				// ACT
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference2[] = {
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(0, 0, 100, 13), 0, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 100, 13), 0, 0, L"lorem"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(100, 0, 117, 13), 1, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(100, 0, 117, 13), 1, 0, L"ipsum"),
				};

				assert_equal_pred(reference2, hdr.events, rect_eq());
			}


			test( ClickOnAHeaderActivatesIt )
			{
				// INIT
				tracking_header hdr;
				column_t c[] = { column_t(L"", 17), column_t(L"", 29), column_t(L"", 25), };
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(c, columns_model::npos(), true);

				hdr.resize(1000, 33, nviews);
				hdr.set_model(m);

				// ACT
				hdr.mouse_up(mouse_input::left, 0, 3, 0);

				// ASSERT
				short int reference1[] = { 0, };

				assert_equal(reference1, m->column_activation_log);

				// ACT
				hdr.mouse_up(mouse_input::left, 0, 46 + 3, 100);

				// ASSERT
				short int reference2[] = { 0, 2, };

				assert_equal(reference2, m->column_activation_log);

				// ACT
				hdr.mouse_up(mouse_input::left, 0, 17 - 4, 10);

				// ASSERT
				short int reference3[] = { 0, 2, 0, };

				assert_equal(reference3, m->column_activation_log);

				// ACT
				hdr.mouse_up(mouse_input::left, 0, 17 + 3, 10);

				// ASSERT
				short int reference4[] = { 0, 2, 0, 1, };

				assert_equal(reference4, m->column_activation_log);

				// INIT
				m->columns[0].width = 27;
				m->columns[1].width = 27;
				m->columns[2].width = 10;

				// INIT
				m->column_activation_log.clear();

				// ACT
				hdr.mouse_up(mouse_input::left, 0, 3, 0);
				hdr.mouse_up(mouse_input::left, 0, 27 - 4, 0);
				hdr.mouse_up(mouse_input::left, 0, 27 + 3, 0);
				hdr.mouse_up(mouse_input::left, 0, 54 - 4, 0);
				hdr.mouse_up(mouse_input::left, 0, 54 + 3, 0);
				hdr.mouse_up(mouse_input::left, 0, 64 - 4, 0);
				hdr.mouse_up(mouse_input::left, 0, 64 + 3, 0); // shall be ignored

				// ASSERT
				short int reference5[] = { 0, 0, 1, 1, 2, 2, };

				assert_equal(reference5, m->column_activation_log);
			}


			test( ClickingIntoColumnGapStartsWidthUpdates )
			{
				// INIT
				tracking_header hdr;
				column_t c[] = { column_t(L"", 17), column_t(L"", 13), column_t(L"", 29), };
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(c, columns_model::npos(), true);

				hdr.resize(1000, 33, nviews);
				hdr.set_model(m);

				// ACT
				hdr.mouse_down(mouse_input::left, 0, 17, 0);
				hdr.mouse_move(mouse_input::left, 16, 10);

				// ASSERT
				assert_equal(16, m->columns[0].width);

				// ACT
				hdr.mouse_move(mouse_input::left, 37, 100);

				// ASSERT
				assert_equal(37, m->columns[0].width);

				// ACT
				hdr.mouse_down(mouse_input::left, 0, 81, 0);

				// ASSERT
				assert_equal(37, m->columns[0].width);
				assert_equal(13, m->columns[1].width);
				assert_equal(29, m->columns[2].width);

				// ACT
				hdr.mouse_move(mouse_input::left, 74, 10);

				// ASSERT
				assert_equal(37, m->columns[0].width);
				assert_equal(22, m->columns[2].width);
			}


			test( MouseIsCapturedOnStartingWidthUpdate )
			{
				// INIT
				tracking_header hdr;
				column_t c[] = { column_t(L"", 17), column_t(L"", 13), column_t(L"", 29), };
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(c, columns_model::npos(), true);
				auto capture = 0;
				auto release = 0;
				auto conn = hdr.capture += [&] (shared_ptr<void> &handle) {
					capture++;
					handle.reset(new int, [&] (int *p) {
						release++;
						delete p;
					});
				};

				hdr.resize(1000, 33, nviews);
				hdr.set_model(m);

				// ACT
				hdr.mouse_down(mouse_input::left, 0, 17, 0);

				// ASSERT
				assert_equal(1, capture);
				assert_equal(0, release);

				// ACT
				hdr.mouse_move(0, 25, 0);

				// ASSERT
				assert_equal(1, capture);
				assert_equal(0, release);

				// ACT
				hdr.mouse_up(mouse_input::left, 0, 25, 0);

				// ASSERT
				assert_equal(1, capture);
				assert_equal(1, release);
			}


			test( WidthUpdatesAreEndedWithMouseUp )
			{
				// INIT
				tracking_header hdr;
				column_t c[] = { column_t(L"", 17), column_t(L"", 13), column_t(L"", 29), };
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(c, columns_model::npos(), true);

				hdr.resize(1000, 33, nviews);
				hdr.set_model(m);

				hdr.mouse_down(mouse_input::left, 0, 17, 0);
				hdr.mouse_move(mouse_input::left, 10, 10);

				// ACT
				hdr.mouse_up(mouse_input::left, 0, 10, 0);
				hdr.mouse_move(0, 100, 10);

				// ASSERT
				assert_equal(10, m->columns[0].width);
			}


			test( ColumnActivationDoesNotOccurWhenResizing )
			{
				// INIT
				tracking_header hdr;
				column_t c[] = { column_t(L"", 17), column_t(L"", 13), column_t(L"", 29), };
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(c, columns_model::npos(), true);

				hdr.resize(1000, 33, nviews);
				hdr.set_model(m);

				// ACT
				hdr.mouse_down(mouse_input::left, 0, 17, 0); // start resizing
				hdr.mouse_down(mouse_input::left, 0, 8, 0); // attempt to click a header

				// ASSERT
				assert_is_empty(m->column_activation_log);
			}


			test( StateIsProvidedToDrawingFunctions )
			{
				// INIT
				tracking_header hdr;
				column_t c[] = { column_t(L"a", 10), column_t(L"b", 13), column_t(L"Z A", 17), };
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(c, 2, true);

				hdr.resize(1000, 33, nviews);
				hdr.set_model(m);

				// ACT
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference1[] = {
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(0, 0, 10, 33), 0, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 10, 33), 0, 0, L"a"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(10, 0, 23, 33), 1, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(10, 0, 23, 33), 1, 0, L"b"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(23, 0, 40, 33), 2,
						controls::header::ascending | controls::header::sorted),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(23, 0, 40, 33), 2,
						controls::header::ascending | controls::header::sorted, L"Z A"),
				};

				assert_equal_pred(reference1, hdr.events, rect_eq());

				// INIT
				hdr.events.clear();

				// ACT
				m->set_sort_order(0, false);
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference2[] = {
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(0, 0, 10, 33), 0, controls::header::sorted),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(0, 0, 10, 33), 0, controls::header::sorted, L"a"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(10, 0, 23, 33), 1, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(10, 0, 23, 33), 1, 0, L"b"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(23, 0, 40, 33), 2, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(23, 0, 40, 33), 2, 0, L"Z A"),
				};

				assert_equal_pred(reference2, hdr.events, rect_eq());
			}


			test( HeaderIsInvalidatedOnSortOrderChange )
			{
				// INIT
				tracking_header hdr;
				auto invalidations = 0;
				column_t c[] = { column_t(L"a", 10), column_t(L"b", 13), column_t(L"Z A", 17), };
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(c, 2, true);
				auto conn = hdr.invalidate += [&] (const agge::rect_i *r) { assert_null(r); invalidations++; };

				hdr.resize(1000, 33, nviews);
				hdr.set_model(m);

				// ACT
				m->set_sort_order(0, true);

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				m->set_sort_order(2, false);
				m->set_sort_order(1, true);

				// ASSERT
				assert_equal(3, invalidations);
			}


			test( OffsettingViewDrawsColumnsAtOffsetPositions )
			{
				// INIT
				tracking_header hdr;
				column_t c[] = { column_t(L"a", 10), column_t(L"b", 13), column_t(L"Z A", 17), };
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(c, 2, true);

				hdr.resize(1000, 33, nviews);
				hdr.set_model(m);

				// ACT
				hdr.set_offset(13.7f);
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference1[] = {
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(-13.7, 0.0, -3.7, 33.0), 0, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(-13.7, 0.0, -3.7, 33.0), 0, 0, L"a"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(-3.7, 0.0, 9.3, 33.0), 1, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(-3.7, 0.0, 9.3, 33.0), 1, 0, L"b"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(9.3, 0.0, 26.3, 33.0), 2,
						controls::header::ascending | controls::header::sorted),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(9.3, 0.0, 26.3, 33.0), 2,
						controls::header::ascending | controls::header::sorted, L"Z A"),
				};

				assert_equal_pred(reference1, hdr.events, rect_eq());

				// INIT
				hdr.events.clear();

				// ACT
				hdr.set_offset(-1.0f);
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference2[] = {
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(1, 0, 11, 33), 0, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(1, 0, 11, 33), 0, 0, L"a"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(11, 0, 24, 33), 1, 0),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(11, 0, 24, 33), 1, 0, L"b"),
					tracking_header::drawing_event(item_background, *ctx, ras, make_rect(24, 0, 41, 33), 2,
						controls::header::ascending | controls::header::sorted),
					tracking_header::drawing_event(item_self, *ctx, ras, make_rect(24, 0, 41, 33), 2,
						controls::header::ascending | controls::header::sorted, L"Z A"),
				};

				assert_equal_pred(reference2, hdr.events, rect_eq());
			}


			test( OffsettingViewProcessesColumnClicksAtNewPositions )
			{
				// INIT
				tracking_header hdr;
				column_t c[] = { column_t(L"a", 10), column_t(L"b", 13), column_t(L"Z A", 17), };
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(c, 2, true);

				hdr.resize(1000, 33, nviews);
				hdr.set_model(m);

				// ACT
				hdr.set_offset(-25.0f);
				hdr.mouse_up(mouse_input::left, 0, 30, 0);
				hdr.mouse_up(mouse_input::left, 0, 41, 0);
				hdr.mouse_up(mouse_input::left, 0, 56, 0);

				// ASSERT
				short int reference[] = { 0, 1, 2, };

				assert_equal(reference, m->column_activation_log);
			}


			test( OffsettingViewInvalidatesIt )
			{
				// INIT
				tracking_header hdr;
				auto invalidations = 0;
				column_t columns[] = { column_t(L"a", 10), column_t(L"b", 13), column_t(L"Z A", 17), };
				shared_ptr<mocks::columns_model> m = mocks::columns_model::create(columns, 2, true);

				hdr.resize(1000, 33, nviews);
				hdr.set_model(mocks::columns_model::create(columns, 2, true));

				auto c = hdr.invalidate += [&] (const void *r) { assert_null(r); invalidations++; };

				// ACT
				hdr.set_offset(-25.0f);

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				hdr.set_offset(-20.0f);
				hdr.set_offset(5.0f);

				// ASSERT
				assert_equal(3, invalidations);
			}

		end_test_suite
	}
}
