#include <wpl/controls/header_core.h>

#include <tests/common/helpers.h>
#include <tests/common/helpers-visual.h>
#include <tests/common/Mockups.h>
#include <tests/common/MockupsListView.h>
#include <tests/common/predicates.h>

#include <ut/assert.h>
#include <ut/test.h>

using namespace std;

namespace wpl
{
	namespace tests
	{
		namespace
		{
			typedef mocks::columns_model::column column_t;

			class tracking_header : public controls::header_core
			{
			public:
				struct drawing_event
				{
					template <typename T>
					drawing_event(gcontext &context_, const gcontext::rasterizer_ptr &rasterizer_, const agge::rect<T> &box_,
							const columns_model &model_, index_type item_, unsigned state_)
						: context(&context_), rasterizer(rasterizer_.get()), model(&model_), item(item_), state(state_)
					{	box.x1 = box_.x1, box.y1 = box_.y1, box.x2 = box_.x2, box.y2 = box_.y2;	}

					gcontext *context;
					gcontext::rasterizer_type *rasterizer;
					agge::rect<double> box;
					const columns_model *model;
					index_type item;
					unsigned state;
				};

				using controls::header_core::item_state_flags;

			public:
				tracking_header(shared_ptr<cursor_manager> cursor_manager_)
					: header_core(cursor_manager_)
				{	}

			public:
				mutable vector<drawing_event> events;
				function<agge::box<int> (const columns_model &model, columns_model::index_type item)> on_measure_item;

			private:
				virtual agge::box<int> measure_item(const columns_model &model, index_type item) const override
				{	return on_measure_item ? on_measure_item(model, item) : agge::zero();	}

				virtual void draw_item(gcontext &ctx, gcontext::rasterizer_ptr &rasterizer, const agge::rect_r &box,
					const columns_model &model, index_type item, unsigned state) const override
				{	events.push_back(drawing_event(ctx, rasterizer, box, model, item, state));	}
			};

			typedef tracking_header::item_state_flags item_state;

			struct rect_eq : eq
			{
				using eq::operator ();

				bool operator ()(const tracking_header::drawing_event &lhs,
					const tracking_header::drawing_event &rhs) const
				{
					return lhs.context == rhs.context && lhs.rasterizer == rhs.rasterizer
						&& (*this)(lhs.box, rhs.box) && lhs.item == rhs.item && lhs.state == rhs.state;
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
			shared_ptr<mocks::cursor_manager> cursor_manager_;

			init( Init )
			{
				surface.reset(new gcontext::surface_type(1000, 1000, 0));
				ren.reset(new gcontext::renderer_type(1));
				text_engine.reset(new gcontext::text_engine_type(fake_loader, 0));
				ctx.reset(new gcontext(*surface, *ren, *text_engine, agge::zero()));
				ras.reset(new gcontext::rasterizer_type);
				cursor_manager_.reset(new mocks::cursor_manager);

				cursor_manager_->cursors[cursor_manager::arrow].reset(new cursor(10, 10, 1, 1));
				cursor_manager_->cursors[cursor_manager::h_resize].reset(new cursor(10, 10, 1, 1));
				cursor_manager_->cursors[cursor_manager::hand].reset(new cursor(10, 10, 1, 1));
			}


			test( ConstructionOfListViewDoesNotDrawAnything )
			{
				// INIT / ACT
				tracking_header hdr(cursor_manager_);

				// ASSERT
				assert_is_empty(hdr.events);
				assert_is_false(provides_tabstoppable_view(hdr));
			}


			test( NothingIsDrawnIfModelsAreMissing )
			{
				// INIT
				tracking_header hdr(cursor_manager_);

				resize(hdr, 1000, 30);

				// ACT
				hdr.draw(*ctx, ras);

				// ASSERT
				assert_is_empty(hdr.events);
			}


			test( MouseEventsDoNothingWhenModelIsNotSet )
			{
				// INIT
				tracking_header hdr(cursor_manager_);

				resize(hdr, 1000, 30);

				// ACT
				hdr.mouse_down(mouse_input::left, 0, 0, 0);
				hdr.mouse_move(mouse_input::left, 0, 0);
				hdr.mouse_up(mouse_input::left, 0, 0, 0);
			}


			test( HeaderIsInvalidateOnModelSet )
			{
				// INIT
				int invalidations = 0;
				tracking_header hdr(cursor_manager_);
				const auto m = mocks::columns_model::create(L"x", 1);
				slot_connection conn = hdr.invalidate += [&] (const agge::rect_i *r) {
					++invalidations;
					assert_null(r);
				};

				resize(hdr, 1000, 30);

				// ACT
				hdr.set_model(m);

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				hdr.set_model(nullptr);

				// ASSERT
				assert_equal(2, invalidations);
			}


			test( ViewInvalidationIsRequestedOnModelInvalidation )
			{
				// INIT
				int invalidations = 0;
				tracking_header hdr(cursor_manager_);
				const auto m = mocks::columns_model::create(L"x", 1);
				slot_connection conn = hdr.invalidate += [&] (const agge::rect_i *r) {
					++invalidations;
					assert_null(r);
				};

				hdr.set_model(m);
				resize(hdr, 1000, 30);
				invalidations = 0;

				// ACT
				m->invalidate();

				// ASSERT
				assert_equal(1, invalidations);

				// ACT
				m->invalidate();
				m->invalidate();

				// ASSERT
				assert_equal(3, invalidations);

				// ACT
				hdr.set_model(nullptr);
				invalidations = 0;
				m->invalidate();
				m->sort_order_changed(1, true);

				// ASSERT
				assert_equal(0, invalidations);
			}


			test( DrawingSequenceIsItemBgThenFg )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				const auto model = mocks::columns_model::create(L"abc", 123);

				resize(hdr, 400, 50);
				hdr.set_model(model);

				// ACT
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference[] = {
					tracking_header::drawing_event(*ctx, ras, create_rect(0, 0, 123, 50), *model, 0, 0),
				};

				assert_equal_pred(reference, hdr.events, rect_eq());
			}


			test( ColumnsAreDrawnInOrder )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c1[] = {	{	L"abc", 10	}, {	L"abc zyx", 17	}, {	L"AA bbb Z", 25	},	};
				auto model = mocks::columns_model::create(c1, columns_model::npos(), true);

				resize(hdr, 1000, 33);
				hdr.set_model(model);

				// ACT
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference1[] = {
					tracking_header::drawing_event(*ctx, ras, create_rect(0, 0, 10, 33), *model, 0, 0),
					tracking_header::drawing_event(*ctx, ras, create_rect(10, 0, 27, 33), *model, 1, 0),
					tracking_header::drawing_event(*ctx, ras, create_rect(27, 0, 52, 33), *model, 2, 0),
				};

				assert_equal_pred(reference1, hdr.events, rect_eq());

				// INIT
				column_t c2[] = {	{	L"lorem", 100	}, {	L"ipsum", 17	},	};
				model = mocks::columns_model::create(c2, columns_model::npos(), true);

				resize(hdr, 1000, 13);
				hdr.set_model(model);
				hdr.events.clear();

				// ACT
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference2[] = {
					tracking_header::drawing_event(*ctx, ras, create_rect(0, 0, 100, 13), *model, 0, 0),
					tracking_header::drawing_event(*ctx, ras, create_rect(100, 0, 117, 13), *model, 1, 0),
				};

				assert_equal_pred(reference2, hdr.events, rect_eq());
			}


			test( ClickOnAHeaderActivatesIt )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 29	}, {	L"", 25	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);

				resize(hdr, 1000, 33);
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
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);

				resize(hdr, 1000, 33);
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


			test( ColumnCannotBeMadeSmallerThanMeasured )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);

				resize(hdr, 1000, 33);
				hdr.set_model(m);

				hdr.on_measure_item = [&] (const columns_model &m_, columns_model::index_type index) -> agge::box<int> {
					assert_equal(m.get(), &m_);
					switch (index)
					{
					case 0:	return agge::create_box(11, 0);
					default:	return agge::create_box(0, 0);
					}
				};

				// ACT
				hdr.mouse_down(mouse_input::left, 0, 17, 0);
				hdr.mouse_move(mouse_input::left, 5, 10);

				// ASSERT
				assert_equal(11, m->columns[0].width);

				// ACT
				hdr.mouse_move(mouse_input::left, 25, 10);

				// ASSERT
				assert_equal(25, m->columns[0].width);

				// INIT
				hdr.mouse_up(mouse_input::left, 0, 25, 10);
				hdr.on_measure_item = [&] (const columns_model &, columns_model::index_type index) -> agge::box<int> {
					switch (index)
					{
					case 2:	return agge::create_box(23, 0);
					default:	return agge::create_box(0, 0);
					}
				};

				// ACT
				hdr.mouse_down(mouse_input::left, 0, 67, 0);
				hdr.mouse_move(mouse_input::left, 0, 10);

				// ASSERT
				assert_equal(23, m->columns[2].width);
			}


			test( MinimumHeightIsZeroOnMissingOrEmptyModel )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				const control &as_control = hdr;
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);

				resize(hdr, 1000, 33);

				// ACT / ASSERT
				assert_equal(0, as_control.min_height());

				// INIT
				hdr.set_model(mocks::columns_model::create());

				// ACT / ASSERT
				assert_equal(0, as_control.min_height());
			}


			test( MinimumHeightIsMaximumOfAllHeadersHeights )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);

				resize(hdr, 1000, 33);
				hdr.set_model(m);

				hdr.on_measure_item = [&] (const columns_model &, columns_model::index_type index) -> agge::box<int> {
					switch (index)
					{
					case 0:	return agge::create_box(10, 5);
					case 1:	return agge::create_box(10, 15);
					case 2:	return agge::create_box(10, 7);
					default:	throw 0;
					}
				};

				// ACT / ASSERT
				assert_equal(15, static_cast<const control &>(hdr).min_height());

				// INIT
				hdr.on_measure_item = [&] (const columns_model &, columns_model::index_type index) -> agge::box<int> {
					switch (index)
					{
					case 0:	return agge::create_box(10, 21);
					case 1:	return agge::create_box(10, 15);
					case 2:	return agge::create_box(10, 7);
					default:	throw 0;
					}
				};

				// ACT / ASSERT
				assert_equal(21, static_cast<const control &>(hdr).min_height());
			}


			test( MouseIsCapturedOnStartingWidthUpdate )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);
				auto capture = 0;
				auto release = 0;
				auto conn = hdr.capture += [&] (shared_ptr<void> &handle) {
					capture++;
					handle.reset(new int, [&] (int *p) {
						release++;
						delete p;
					});
				};

				resize(hdr, 1000, 33);
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


			test( ResizeModeIsCancelledOnModelReset )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);
				auto capture = 0;
				auto conn = hdr.capture += [&] (shared_ptr<void> &handle) {
					capture++;
					handle.reset(new int, [&] (int *p) {
						capture--;
						delete p;
					});
				};

				resize(hdr, 1000, 33);
				hdr.set_model(m);
				hdr.mouse_down(mouse_input::left, 0, 17, 0);

				// ACT
				hdr.set_model(nullptr);

				// ASSERT
				assert_equal(0, capture);

				// ACT
				hdr.mouse_move(0, 20, 0);

				// ASSERT
				assert_equal(17, m->columns[0].width);
			}


			test( WidthUpdatesAreEndedWithMouseUp )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);

				resize(hdr, 1000, 33);
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
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);

				resize(hdr, 1000, 33);
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
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"a", 10	}, {	L"b", 13	}, {	L"Z A", 17	},	};
				const auto m = mocks::columns_model::create(c, 2, true);

				resize(hdr, 1000, 33);
				hdr.set_model(m);

				// ACT
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference1[] = {
					tracking_header::drawing_event(*ctx, ras, create_rect(0, 0, 10, 33), *m, 0, 0),
					tracking_header::drawing_event(*ctx, ras, create_rect(10, 0, 23, 33), *m, 1, 0),
					tracking_header::drawing_event(*ctx, ras, create_rect(23, 0, 40, 33), *m, 2,
						controls::header_core::ascending | controls::header_core::sorted),
				};

				assert_equal_pred(reference1, hdr.events, rect_eq());

				// INIT
				hdr.events.clear();

				// ACT
				m->set_sort_order(0, false);
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference2[] = {
					tracking_header::drawing_event(*ctx, ras, create_rect(0, 0, 10, 33), *m, 0, controls::header_core::sorted),
					tracking_header::drawing_event(*ctx, ras, create_rect(10, 0, 23, 33), *m, 1, 0),
					tracking_header::drawing_event(*ctx, ras, create_rect(23, 0, 40, 33), *m, 2, 0),
				};

				assert_equal_pred(reference2, hdr.events, rect_eq());
			}


			test( HeaderIsInvalidatedOnSortOrderChange )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				auto invalidations = 0;
				column_t c[] = {	{	L"a", 10	}, {	L"b", 13	}, {	L"Z A", 17	},	};
				const auto m = mocks::columns_model::create(c, 2, true);
				auto conn = hdr.invalidate += [&] (const agge::rect_i *r) { assert_null(r); invalidations++; };

				resize(hdr, 1000, 33);
				hdr.set_model(m);
				invalidations = 0;

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
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"a", 10	}, {	L"b", 13	}, {	L"Z A", 17	},	};
				const auto m = mocks::columns_model::create(c, 2, true);

				resize(hdr, 1000, 33);
				hdr.set_model(m);

				// ACT
				hdr.set_offset(13.7f);
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference1[] = {
					tracking_header::drawing_event(*ctx, ras, create_rect(-13.7, 0.0, -3.7, 33.0), *m, 0, 0),
					tracking_header::drawing_event(*ctx, ras, create_rect(-3.7, 0.0, 9.3, 33.0), *m, 1, 0),
					tracking_header::drawing_event(*ctx, ras, create_rect(9.3, 0.0, 26.3, 33.0), *m, 2,
						controls::header_core::ascending | controls::header_core::sorted),
				};

				assert_equal_pred(reference1, hdr.events, rect_eq());

				// INIT
				hdr.events.clear();

				// ACT
				hdr.set_offset(-1.0f);
				hdr.draw(*ctx, ras);

				// ASSERT
				tracking_header::drawing_event reference2[] = {
					tracking_header::drawing_event(*ctx, ras, create_rect(1, 0, 11, 33), *m, 0, 0),
					tracking_header::drawing_event(*ctx, ras, create_rect(11, 0, 24, 33), *m, 1, 0),
					tracking_header::drawing_event(*ctx, ras, create_rect(24, 0, 41, 33), *m, 2,
						controls::header_core::ascending | controls::header_core::sorted),
				};

				assert_equal_pred(reference2, hdr.events, rect_eq());
			}


			test( OffsettingViewProcessesColumnClicksAtNewPositions )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"a", 10	}, {	L"b", 13	}, {	L"Z A", 17	},	};
				const auto m = mocks::columns_model::create(c, 2, true);

				resize(hdr, 1000, 33);
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
				tracking_header hdr(cursor_manager_);
				auto invalidations = 0;
				column_t columns[] = {	{	L"a", 10	}, {	L"b", 13	}, {	L"Z A", 17	},	};

				resize(hdr, 1000, 33);
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


			test( CursorIsSetOnMouseEnterLeave )
			{
				// INIT
				tracking_header hdr(cursor_manager_);

				cursor_manager_->cursors[cursor_manager::arrow].reset(new cursor(10, 10, 1, 1));

				// ACT
				hdr.mouse_enter();

				// ASSERT
				assert_equal(1u, cursor_manager_->stack_level);
				assert_equal(cursor_manager_->cursors[cursor_manager::arrow], cursor_manager_->recently_set);

				// ACT
				hdr.mouse_leave();

				// ASSERT
				assert_equal(0u, cursor_manager_->stack_level);
			}


			test( CursorIsSetAccordinglyToPosition )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t columns[] = {	{	L"a", 10	}, {	L"b", 13	}, {	L"Z A", 17	},	};

				resize(hdr, 1000, 33);
				hdr.set_model(mocks::columns_model::create(columns, 2, true));
				hdr.set_offset(-10);

				// ACT
				hdr.mouse_move(0, 0, 5);

				// ASSERT
				assert_equal(cursor_manager_->cursors[cursor_manager::arrow], cursor_manager_->recently_set);
				assert_equal(0u, cursor_manager_->stack_level);

				// ACT
				hdr.mouse_move(0, 20, 0);

				// ASSERT
				assert_equal(cursor_manager_->cursors[cursor_manager::h_resize], cursor_manager_->recently_set);
				assert_equal(0u, cursor_manager_->stack_level);

				// ACT
				hdr.mouse_move(0, 27, 0);

				// ASSERT
				assert_equal(cursor_manager_->cursors[cursor_manager::hand], cursor_manager_->recently_set);
				assert_equal(0u, cursor_manager_->stack_level);
			}


			test( ColumnHeaderWidthsAreAdjustedOnRequest )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);

				resize(hdr, 1000, 33);
				hdr.set_model(m);

				hdr.on_measure_item = [&] (const columns_model &/*m*/, columns_model::index_type item) -> agge::box<int> {
					switch (item)
					{
					case 0:	return agge::create_box(10, 0);
					case 1:	 return agge::create_box(200, 0);
					case 2:	 return agge::create_box(30, 0);
					case 3: default:	return agge::create_box(29, 0);
					}
				};

				// ACT
				hdr.adjust_column_widths();

				// ACT
				short reference1_ulog[] = {	1, 2,	};
				column_t reference1_widths[] = {	{	L"", 17	}, {	L"", 200	}, {	L"", 30	}, {	L"", 29	},	};

				assert_equal(reference1_ulog, m->column_update_log);
				assert_equal(reference1_widths, m->columns);

				// INIT
				hdr.on_measure_item = [&] (const columns_model &/*m*/, columns_model::index_type item) -> agge::box<int> {
					switch (item)
					{
					case 0:	return agge::create_box(100, 0);
					default:	return agge::create_box(0, 0);
					}
				};
				m->column_update_log.clear();

				// ACT
				hdr.adjust_column_widths();

				// ACT
				short reference2_ulog[] = {	0,	};
				column_t reference2_widths[] = {	{	L"", 100	}, {	L"", 200	}, {	L"", 30	}, {	L"", 29	},	};

				assert_equal(reference2_ulog, m->column_update_log);
				assert_equal(reference2_widths, m->columns);
			}


			test( ColumnWidthsAreAdjustedOnModelSetting )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);

				resize(hdr, 1000, 33);

				hdr.on_measure_item = [&] (const columns_model &/*m*/, columns_model::index_type item) -> agge::box<int> {
					switch (item)
					{
					case 0:	return agge::create_box(100, 0);
					case 2:	 return agge::create_box(72, 0);
					default:	return agge::create_box(0, 0);
					}
				};

				// ACT
				hdr.set_model(m);

				// ACT
				column_t reference_widths[] = {	{	L"", 100	}, {	L"", 13	}, {	L"", 72	}, {	L"", 29	},	};

				assert_equal(reference_widths, m->columns);
			}


			test( ColumnWidthsAreAdjustedOnModelInvalidation )
			{
				// INIT
				tracking_header hdr(cursor_manager_);
				column_t c[] = {	{	L"", 17	}, {	L"", 13	}, {	L"", 29	}, {	L"", 29	},	};
				const auto m = mocks::columns_model::create(c, columns_model::npos(), true);

				m->invalidate_on_update = true;
				resize(hdr, 1000, 33);
				hdr.set_model(m);

				hdr.on_measure_item = [&] (const columns_model &/*m*/, columns_model::index_type item) -> agge::box<int> {
					switch (item)
					{
					case 1:	return agge::create_box(153, 0);
					case 3:	return agge::create_box(31, 0);
					default:	return agge::create_box(0, 0);
					}
				};

				// ACT
				m->invalidate();

				// ACT
				short reference1_ulog[] = {	1, 3,	};
				column_t reference1_widths[] = {	{	L"", 17	}, {	L"", 153	}, {	L"", 29	}, {	L"", 31	},	};

				assert_equal(reference1_ulog, m->column_update_log);
				assert_equal(reference1_widths, m->columns);

				// INIT
				hdr.on_measure_item = [&] (const columns_model &/*m*/, columns_model::index_type item) -> agge::box<int> {
					switch (item)
					{
					case 2:	return agge::create_box(32, 0);
					default:	return agge::create_box(0, 0);
					}
				};

				// ACT
				m->invalidate();

				// ACT
				column_t reference2_widths[] = {	{	L"", 17	}, {	L"", 153	}, {	L"", 32	}, {	L"", 31	},	};

				assert_equal(reference2_widths, m->columns);
			}

		end_test_suite
	}
}
